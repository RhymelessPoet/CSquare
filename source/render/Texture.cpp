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

} // namespace CS