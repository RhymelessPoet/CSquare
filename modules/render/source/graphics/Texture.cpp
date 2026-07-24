#include "Texture.h"
#include "GraphicsResourceDescriptors.h"
#include "RenderTarget.h"
#include "graphics/GraphicsAPI.h"

namespace CS
{
Texture::Texture(TextureDescriptor* descriptor) : IGraphicsResource(descriptor) {}

std::any Texture::GetNativeTexture() const
{
    return descriptor<TextureDescriptor>().GetNativeTexture();
}

void Texture::SetNativeTexture(std::any nativeTexture)
{
    auto& textureDescriptor = descriptor<TextureDescriptor>();
    textureDescriptor.SetExternalTexture(std::any_cast<uint32_t>(nativeTexture));
}

void Texture::SetSize(const Size2u& size)
{
    auto& textureDescriptor = descriptor<TextureDescriptor>();
    textureDescriptor.SetSize(size);
}

Size2u Texture::GetSize() const
{
    return descriptor<TextureDescriptor>().GetSize();
}

void Texture::SetFormat(TextureFormat format)
{
    descriptor<TextureDescriptor>().SetFormat(format);
}

TextureFormat Texture::GetFormat() const
{
    return descriptor<TextureDescriptor>().GetFormat();
}

void Texture::SetMipmap(bool mipmap)
{
    descriptor<TextureDescriptor>().SetMipmap(mipmap);
}

bool Texture::IsMipmap() const
{
    return descriptor<TextureDescriptor>().IsMipmap();
}

void Texture::UpdateData(const void* data, const Size2u& size)
{
    auto& desc = descriptor<TextureDescriptor>();
    desc.SetSize(size, false);
    desc.UpdateData(data);
}

} // namespace CS