#include "MaterialTexturesMap.h"
#include "asset/Image.h"
#include "graphics/GraphicsAPI.h"
#include "graphics/Sampler.h"
#include "materials/ImageTexture.h"

namespace CS
{
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
        sampledTexture.texture.UpdateData(reinterpret_cast<const void*>(image->GetData()), image->GetSize().XY());
    }
}

} // namespace CS
