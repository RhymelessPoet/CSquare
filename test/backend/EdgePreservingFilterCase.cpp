#include "EdgePreservingFilterCase.h"
#include "graphics/GLRendererBuilder.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/GraphicsResourceCache.h"
#include "graphics/GraphicsShaderStage.h"
#include "graphics/ShaderBinding.h"
#include "materials/Shader.h"
#include "renderer/Renderer.h"
#include <QCoreApplication>
#include <QDir>

#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifdef MemoryBarrier
#undef MemoryBarrier
#endif
#endif

namespace
{
constexpr const char* BilateralShader = R"glsl(
#version 430 core
layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0) uniform sampler2D sourceImage;
layout(rgba8, binding = 1) writeonly uniform image2D resultImage;

void main()
{
    ivec2 p = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = textureSize(sourceImage, 0);
    if (any(greaterThanEqual(p, size))) return;
    vec3 center = texelFetch(sourceImage, p, 0).rgb;
    vec3 sum = vec3(0.0);
    float total = 0.0;
    const float spatialSigma = 2.0;
    const float rangeSigma = 0.12;
    for (int y = -3; y <= 3; ++y) {
        for (int x = -3; x <= 3; ++x) {
            ivec2 q = clamp(p + ivec2(x, y), ivec2(0), size - 1);
            vec3 sampleColor = texelFetch(sourceImage, q, 0).rgb;
            float spatial = exp(-float(x*x + y*y) / (2.0 * spatialSigma * spatialSigma));
            vec3 delta = sampleColor - center;
            float range = exp(-dot(delta, delta) / (2.0 * rangeSigma * rangeSigma));
            float weight = spatial * range;
            sum += sampleColor * weight;
            total += weight;
        }
    }
    imageStore(resultImage, p, vec4(sum / max(total, 0.00001), 1.0));
}
)glsl";
}

EdgePreservingFilterCase::EdgePreservingFilterCase() = default;
EdgePreservingFilterCase::~EdgePreservingFilterCase() = default;

bool EdgePreservingFilterCase::initialize(QString& error)
{
    if (m_renderer)
        return true;
#ifdef Q_OS_WIN
    auto currentContext = reinterpret_cast<void*>(wglGetCurrentContext());
    if (!currentContext) {
        error = QStringLiteral("Qt render thread has no current OpenGL context");
        return false;
    }
    m_cache = std::make_shared<CS::GraphicsResourceCache>();
    CS::GLRendererBuilder builder;
    builder.SetSharedContext(currentContext);
    m_renderer = builder.Build(m_cache);
    m_renderer->GetGraphicsAPI()->Initialize();
    return true;
#else
    error = QStringLiteral("Compute test currently supports Windows OpenGL only");
    return false;
#endif
}

QImage EdgePreservingFilterCase::run(QString& error)
{
    if (!initialize(error)) {
        return {};
    }
    const QString path = QDir(QCoreApplication::applicationDirPath()).filePath("assets/images/lighthouse.png");
    QImage source(path);
    if (source.isNull()) {
        error = QStringLiteral("Failed to load %1").arg(path);
        return {};
    }
    source = source.convertToFormat(QImage::Format_RGBA8888).mirrored(false, true);

    auto api = m_renderer->GetGraphicsAPI();
    const CS::Size2u size{uint32_t(source.width()), uint32_t(source.height())};
    auto input = api->CreateTexture(CS::TextureFormat::RGBA8Unorm, size);
    input.Build();
    input.UpdateData(source.constBits(), size);
    auto output = api->CreateTexture(CS::TextureFormat::RGBA8Unorm, size);
    output.Build();

    auto sampler = api->CreateSampler();
    sampler.Build();
    auto layout = api->CreateShaderBindingSetLayout();
    layout.AddBinding(CS::ShaderBinding(0, CS::ShaderStage::Compute, CS::ShaderBinding::Type::SampledTexture));
    layout.AddBinding(CS::ShaderBinding(1, CS::ShaderStage::Compute, CS::ShaderBinding::Type::StorageTexture));
    auto bindings = api->CreateShaderBindingSet(layout);
    if (!bindings.BindSampledTexture(0, input, sampler) ||
        !bindings.BindStorageTexture(1, output, CS::StorageTextureAccess::WriteOnly))
    {
        error = QStringLiteral("Failed to bind compute textures");
        return {};
    }

    auto pipeline = api->CreateComputePipeline();
    auto shader = std::make_shared<CS::Shader>(BilateralShader, CS::ShaderStage::Compute);
    pipeline.SetShaderStage(std::make_unique<CS::GraphicsShaderStage>(shader));
    auto commands = api->CreateCommandBuffer();
    commands.Bind(pipeline)
        .Bind(bindings)
        .Dispatch((size.width + 15) / 16, (size.height + 15) / 16, 1)
        .Barrier(CS::MemoryBarrier::StorageTexture | CS::MemoryBarrier::TextureFetch);
    try {
        api->SubmitCommandBuffer(commands);
    } catch (const std::exception& e) {
        error = QString::fromUtf8(e.what());
        return {};
    }

    auto pixels = api->ReadTexture(output);
    if (pixels.size() != size_t(size.width) * size.height * 4) {
        error = QStringLiteral("Unexpected compute output size");
        return {};
    }
    QImage result(reinterpret_cast<const uchar*>(pixels.data()), int(size.width), int(size.height),
                  QImage::Format_RGBA8888);
    return result.copy().mirrored(false, true);
}
