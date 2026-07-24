#pragma once
#include "GraphicsResourceParameters.h"
#include "IGraphicsResource.h"
#include "Size.h"
#include <any>

namespace CS
{
class TextureDescriptor;

class Texture final : public IGraphicsResource
{
public:
    friend class GraphicsAPI;
    Texture() = default;
    ~Texture() = default;

    std::any GetNativeTexture() const;
    void SetNativeTexture(std::any nativeTexture);

    void SetSize(const Size2u& size);
    Size2u GetSize() const;

    void SetFormat(TextureFormat format);
    TextureFormat GetFormat() const;

    void SetMipmap(bool mipmap);
    bool IsMipmap() const;

    void UpdateData(const void* data, const Size2u& size);

private:
    Texture(TextureDescriptor* descriptor);
};

} // namespace CS
