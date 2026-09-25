#include "LaplacianSmoothingCase.h"
#include "graphics/GLRendererBuilder.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/GraphicsResourceCache.h"
#include "graphics/GraphicsShaderStage.h"
#include "graphics/ShaderBinding.h"
#include "graphics/VertexInputLayout.h"
#include "materials/Shader.h"
#include "renderer/Renderer.h"
#include <cmath>
#include <span>

#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifdef MemoryBarrier
#undef MemoryBarrier
#endif
#endif

namespace
{
constexpr uint32_t GridSize = 96;
constexpr uint32_t Width = 800;
constexpr uint32_t Height = 600;

constexpr const char* SmoothShader = R"glsl(
#version 430 core
layout(local_size_x = 16, local_size_y = 16) in;
layout(std430, binding = 0) readonly buffer InputPositions { vec4 inputPosition[]; };
layout(std430, binding = 1) writeonly buffer OutputPositions { vec4 outputPosition[]; };
const uint gridSize = 96u;
void main() {
    uvec2 p = gl_GlobalInvocationID.xy;
    if (p.x >= gridSize || p.y >= gridSize) {
        return;
    }

    uint i = p.y * gridSize + p.x;
    if (p.x == 0u || p.y == 0u || p.x + 1u == gridSize || p.y + 1u == gridSize) {
        outputPosition[i] = inputPosition[i];
        return;
    }

    vec4 average = (inputPosition[i-1u] + inputPosition[i+1u] +
                    inputPosition[i-gridSize] + inputPosition[i+gridSize]) * 0.25;

    outputPosition[i] = mix(inputPosition[i], average, 0.55);
}
)glsl";

constexpr const char* VertexShader = R"glsl(
#version 430 core
layout(location = 0) in vec4 position;
out vec3 viewPosition;
void main() {
    float cx = 0.8660254, sx = 0.5;
    float cy = 0.9063078, sy = 0.4226183;
    vec3 p = position.xyz;
    vec3 rx = vec3(p.x, cx*p.y-sx*p.z, sx*p.y+cx*p.z);
    vec3 v = vec3(cy*rx.x + sy*rx.z, rx.y, -sy*rx.x + cy*rx.z);
    viewPosition = v;
    gl_Position = vec4(v.x * 0.78, v.y * 0.92, (v.z + 2.0) / 4.0, 1.0);
}
)glsl";

constexpr const char* FragmentShader = R"glsl(
#version 430 core
in vec3 viewPosition;
out vec4 fragColor;
void main() {
    vec3 n = normalize(cross(dFdx(viewPosition), dFdy(viewPosition)));
    if (n.z < 0.0) n = -n;
    vec3 l = normalize(vec3(-0.35, 0.55, 1.0));
    vec3 v = normalize(vec3(0.0, 0.0, 2.0) - viewPosition);
    vec3 h = normalize(l + v);
    float diffuse = max(dot(n, l), 0.0);
    float specular = pow(max(dot(n, h), 0.0), 48.0);
    vec3 base = vec3(0.18, 0.48, 0.78);
    fragColor = vec4(base * (0.18 + 0.82 * diffuse) + vec3(0.55) * specular, 1.0);
}
)glsl";

struct Position
{
    float x, y, z, w;
};
} // namespace

LaplacianSmoothingCase::LaplacianSmoothingCase() = default;
LaplacianSmoothingCase::~LaplacianSmoothingCase() = default;

bool LaplacianSmoothingCase::initialize(QString& error)
{
    if (m_renderer)
        return true;
#ifdef Q_OS_WIN
    auto context = reinterpret_cast<void*>(wglGetCurrentContext());
    if (!context) {
        error = QStringLiteral("Qt render thread has no current OpenGL context");
        return false;
    }
    m_cache = std::make_shared<CS::GraphicsResourceCache>();
    CS::GLRendererBuilder builder;
    builder.SetSharedContext(context);
    m_renderer = builder.Build(m_cache);
    m_renderer->GetGraphicsAPI()->Initialize();
    return true;
#else
    error = QStringLiteral("Compute test currently supports Windows OpenGL only");
    return false;
#endif
}

QImage LaplacianSmoothingCase::run(QString& error)
{
    if (!initialize(error)) {
        return {};
    }
    auto api = m_renderer->GetGraphicsAPI();
    std::vector<Position> positions(GridSize * GridSize);
    for (uint32_t y = 0; y < GridSize; ++y)
        for (uint32_t x = 0; x < GridSize; ++x) {
            float px = float(x) / float(GridSize - 1) * 2.0f - 1.0f;
            float py = float(y) / float(GridSize - 1) * 2.0f - 1.0f;
            float r = std::sqrt(px * px + py * py);
            float z = 0.22f * std::sin(px * 10.0f) * std::cos(py * 9.0f) * std::exp(-r * r * 1.2f);
            z += 0.01 * (x % 2 == 0 ? 1.0f : -1.0f) * (y % 2 == 0 ? 1.0f : -1.0f);
            positions[y * GridSize + x] = {px, py, z, 1.0f};
        }
    std::vector<uint32_t> indices;
    indices.reserve((GridSize - 1) * (GridSize - 1) * 6);
    for (uint32_t y = 0; y + 1 < GridSize; ++y)
        for (uint32_t x = 0; x + 1 < GridSize; ++x) {
            uint32_t a = y * GridSize + x, b = a + 1, c = a + GridSize, d = c + 1;
            indices.insert(indices.end(), {a, c, b, b, c, d});
        }

    const size_t vertexBytes = positions.size() * sizeof(Position);
    auto input = api->CreateStorageBuffer(vertexBytes);
    input.Build();
    input.UpdateData(positions.data(), vertexBytes);
    auto output = api->CreateStorageBuffer(vertexBytes);
    output.Build();
    auto computeLayout = api->CreateShaderBindingSetLayout();
    computeLayout.AddBinding(CS::ShaderBinding(0, CS::ShaderStage::Compute, CS::ShaderBinding::Type::StorageBuffer));
    computeLayout.AddBinding(CS::ShaderBinding(1, CS::ShaderStage::Compute, CS::ShaderBinding::Type::StorageBuffer));
    auto computeBindings = api->CreateShaderBindingSet(computeLayout);
    if (!computeBindings.BindStorageBuffer(0, input, 0, vertexBytes) ||
        !computeBindings.BindStorageBuffer(1, output, 0, vertexBytes))
    {
        error = QStringLiteral("Failed to bind smoothing buffers");
        return {};
    }
    auto compute = api->CreateComputePipeline();
    compute.SetShaderStage(std::make_unique<CS::GraphicsShaderStage>(
        std::make_shared<CS::Shader>(SmoothShader, CS::ShaderStage::Compute)));

    auto vertexLayout = std::make_shared<CS::VertexInputLayout>();
    vertexLayout->SetBinding(0, CS::VertexInputBinding(sizeof(Position), CS::VertexInputRate::PerVertex));
    vertexLayout->SetAttribute(0, CS::VertexInputAttribute(0, CS::VertexInputFormat::Float4, 0));
    auto assembly = api->CreateInputAssembly();
    assembly.SetVertexInputLayout(vertexLayout);
    assembly.SetVertexInput(0, api->CreateVertexBufferView(input), 0);
    // assembly.SetVertexInput(0, api->CreateVertexBufferView(output), 0);
    auto indexBuffer = api->CreateIndexBuffer(indices.size() * sizeof(uint32_t));
    indexBuffer.Build();
    indexBuffer.UpdateData(indices.data(), indices.size() * sizeof(uint32_t));
    assembly.SetIndexBuffer(indexBuffer, uint32_t{});

    auto graphics = api->CreatePipeline();
    graphics.SetVertexInputLayout(vertexLayout).SetDepthTest(true).SetCullMode(CS::CullMode::None);
    graphics.SetShaderStage(
        std::make_unique<CS::GraphicsShaderStage>(std::make_shared<CS::Shader>(VertexShader, CS::ShaderStage::Vertex)));
    graphics.SetShaderStage(std::make_unique<CS::GraphicsShaderStage>(
        std::make_shared<CS::Shader>(FragmentShader, CS::ShaderStage::Fragment)));
    CS::Size2u renderSize{Width, Height};
    auto color = api->CreateTexture(CS::TextureFormat::RGBA8Unorm, renderSize);
    auto depth = api->CreateTexture(CS::TextureFormat::Depth32, renderSize);
    auto target = api->CreateRenderTarget(renderSize);
    target.SetColorAttachment(color);
    target.SetDepthAttachment(depth);

    auto commands = api->CreateCommandBuffer();
    commands.Bind(compute)
        .Bind(computeBindings)
        .Dispatch((GridSize + 15) / 16, (GridSize + 15) / 16, 1)
        .Barrier(CS::MemoryBarrier::StorageBuffer | CS::MemoryBarrier::VertexBuffer)
        .BeginPass(target)
        .SetViewport(0, 0, Width, Height)
        .Clear(CS::Color(0.035f, 0.045f, 0.06f, 1.0f), 1.0f)
        .Bind(graphics)
        .Bind(assembly)
        .DrawIndexed(uint32_t(indices.size()), 0)
        .EndPass();
    try {
        api->SubmitCommandBuffer(commands);
    } catch (const std::exception& e) {
        error = QString::fromUtf8(e.what());
        return {};
    }
    auto pixels = api->ReadTexture(color);
    if (pixels.size() != size_t(Width) * Height * 4) {
        error = QStringLiteral("Unexpected render output size");
        return {};
    }
    QImage image(reinterpret_cast<const uchar*>(pixels.data()), Width, Height, QImage::Format_RGBA8888);
    return image.copy().mirrored(false, true);
}
