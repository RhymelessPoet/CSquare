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
    if (!textureDescriptor.IsBuild()) {
        textureDescriptor.SetNativeTexture(std::any_cast<uint32_t>(nativeTexture));
    }
}

} // namespace CS