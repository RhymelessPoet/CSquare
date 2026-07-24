#pragma once
#include "Macros.h"
#include "asset/Image.h"
#include "graphics/Sampler.h"
#include "graphics/Texture.h"
#include <map>
#include <string>


namespace CS
{
class ImageTexture;
class RenderTexture;
class GraphicsAPI;

class MaterialTexturesMap
{
public:
    struct SampledTexture
    {
        Texture texture;
        Sampler sampler;
    };
    MaterialTexturesMap(std::shared_ptr<GraphicsAPI> graphicsAPI);
    ~MaterialTexturesMap();

    CS_DELETE_COPY_MOVE(MaterialTexturesMap);

    SampledTexture* AllocateTexture(std::string_view name, const ImageTexture& texture);
    SampledTexture* AllocateTexture(std::string_view name, const RenderTexture& renderTexture, Texture texture);
    SampledTexture* GetTexture(std::string_view name);

    void SetTextureData(std::string_view name, std::shared_ptr<Image> image);

    void UpdateTextures();

private:
    std::shared_ptr<GraphicsAPI> m_graphicsAPI;
    std::map<std::string, SampledTexture> m_textures;
    std::map<std::string, std::shared_ptr<Image>> m_toUpdateImages;
};

} // namespace CS
