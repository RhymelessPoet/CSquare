#include "MaterialTexturesMap.h"
#include "asset/Image.h"
#include "base/Logger.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/RenderTexture.h"
#include "graphics/Sampler.h"
#include "materials/ImageTexture.h"

namespace CS
{
static inline TextureFormat GetTextureFormatFromImageFormat(ImageFormat format, bool isSRGB = false)
{
    switch (format) {
    case ImageFormat::RGBA8:
        return isSRGB ? TextureFormat::RGBA8Srgb : TextureFormat::RGBA8Unorm;
    case ImageFormat::RGB8:
        return isSRGB ? TextureFormat::RGB8Srgb : TextureFormat::RGB8Unorm;
    case ImageFormat::RGB32Float:
        return TextureFormat::RGB32Float;
    case ImageFormat::RGBA32Float:
        return TextureFormat::RGBA32Float;
    default:
        return TextureFormat::RGBA8Unorm;
    }
}

MaterialTexturesMap::MaterialTexturesMap(std::shared_ptr<GraphicsAPI> graphicsAPI)
    : m_graphicsAPI(std::move(graphicsAPI))
{}

MaterialTexturesMap::~MaterialTexturesMap() {}

MaterialTexturesMap::SampledTexture* MaterialTexturesMap::AllocateTexture(std::string_view name,
                                                                          const ImageTexture& imageTexture)
{
    auto nameStr = std::string(name);
    auto itr = m_textures.find(nameStr);
    if (itr != m_textures.end()) {
        return &itr->second;
    }

    auto texture = m_graphicsAPI->CreateTexture();

    auto sampler = m_graphicsAPI->CreateSampler();
    auto [u, v] = imageTexture.GetAddressModeUV();
    sampler.SetAddressModeUV(u, v);

    auto [minFilter, magFilter] = imageTexture.GetFilter();
    sampler.SetFilter(minFilter, magFilter);

    if (imageTexture.GetMipmapFilter() != MipmapFilterMode::Max) {
        sampler.SetMipmapFilter(imageTexture.GetMipmapFilter());
    }

    const auto& image = imageTexture.GetImage();
    if (image != nullptr) {
        auto format = GetTextureFormatFromImageFormat(image->GetFormat(), imageTexture.IsSRGB());
        texture.SetFormat(format);
        texture.SetMipmap(imageTexture.UseMipmaps());
        m_toUpdateImages[nameStr] = image;
    }

    m_textures[nameStr] = SampledTexture{.texture = texture, .sampler = sampler};

    return &m_textures[nameStr];
}

MaterialTexturesMap::SampledTexture*
MaterialTexturesMap::AllocateTexture(std::string_view name, const RenderTexture& renderTexture, Texture texture)
{
    auto nameStr = std::string(name);
    auto itr = m_textures.find(nameStr);
    if (itr != m_textures.end()) {
        return &itr->second;
    }
    auto sampler = m_graphicsAPI->CreateSampler();
    auto [u, v] = renderTexture.GetAddressModeUV();
    sampler.SetAddressModeUV(u, v);

    auto [minFilter, magFilter] = renderTexture.GetFilter();
    sampler.SetFilter(minFilter, magFilter);

    if (renderTexture.GetMipmapFilter() != MipmapFilterMode::Max) {
        sampler.SetMipmapFilter(renderTexture.GetMipmapFilter());
    }

    m_textures[nameStr] = SampledTexture{.texture = texture, .sampler = sampler};

    return &m_textures[nameStr];
}

MaterialTexturesMap::SampledTexture* MaterialTexturesMap::GetTexture(std::string_view name)
{
    auto it = m_textures.find(std::string(name));
    if (it != m_textures.end()) {
        return &it->second;
    }
    return nullptr;
}

void MaterialTexturesMap::SetTextureData(std::string_view name, std::shared_ptr<Image> image)
{
    if (image == nullptr) {
        return;
    }
    auto it = m_textures.find(std::string(name));
    if (it != m_textures.end()) {
        m_toUpdateImages[std::string(name)] = image;
    } else {
        CS::LogError(::CS::BuiltInChannels::Render(),
                     CS::Fmt("SetTextureData: texture '{}' not found", std::string(name)));
    }
}

void MaterialTexturesMap::UpdateTextures()
{
    for (auto& [name, sampledTexture] : m_textures) {
        // Sampler state is immutable after creation; only build once.
        if (!sampledTexture.sampler.IsBuild()) {
            sampledTexture.sampler.Build();
        }

        auto itr = m_toUpdateImages.find(name);
        if (itr == m_toUpdateImages.end() || itr->second == nullptr) {
            continue;
        }
        auto& image = itr->second;

        if (!sampledTexture.texture.IsBuild()) {
            sampledTexture.texture.Build();
        }
        sampledTexture.texture.UpdateData(reinterpret_cast<const void*>(image->GetData()), image->GetSize().XY());
    }
    m_toUpdateImages.clear();
}

} // namespace CS