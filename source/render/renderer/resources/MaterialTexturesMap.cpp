#include "MaterialTexturesMap.h"
#include "asset/Image.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/Sampler.h"
#include "materials/ImageTexture.h"

namespace CS
{
static inline TextureFormat GetTextureFormatFromImageFormat(ImageFormat format)
{
    switch (format) {
    case ImageFormat::RGBA8:
        return TextureFormat::RGBA8Unorm;
    case ImageFormat::RGB8:
        return TextureFormat::RGB8Unorm;
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

    m_textures[nameStr] = SampledTexture{.texture = texture, .sampler = sampler};
    m_toUpdateImages[nameStr] = imageTexture.GetImage();

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
    auto it = m_textures.find(std::string(name));
    if (it != m_textures.end()) {
        m_toUpdateImages[std::string(name)] = image;
    } else {
        // TODO: log error
    }
}

void MaterialTexturesMap::UpdateTextures()
{
    for (auto& [name, sampledTexture] : m_textures) {
        sampledTexture.texture.Build();
        sampledTexture.sampler.Build();

        auto itr = m_toUpdateImages.find(name);
        if (itr == m_toUpdateImages.end()) {
            continue;
        }
        auto& image = itr->second;
        auto format = GetTextureFormatFromImageFormat(image->GetFormat());
        sampledTexture.texture.SetFormat(format);
        sampledTexture.texture.Build();
        sampledTexture.texture.UpdateData(reinterpret_cast<const void*>(image->GetData()), image->GetSize().XY());
    }
    m_toUpdateImages.clear();
}

} // namespace CS
