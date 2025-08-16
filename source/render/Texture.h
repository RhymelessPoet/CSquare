#pragma once
#include "IGraphicsResource.h"
#include "base/Size.h"
#include <any>

namespace CS
{
class TextureDescriptor;

class Texture final : public IGraphicsResource
{
public:
    friend class GraphicsAPI;
    ~Texture() = default;

    std::any GetNativeTexture() const;
    void SetNativeTexture(std::any nativeTexture);

    void SetSize(const Size2U& size);
    Size2U GetSize() const;

private:
    Texture(TextureDescriptor* descriptor);
};

} // namespace CS
