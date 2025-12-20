#pragma once
#include "IGraphicsResource.h"
#include "Texture.h"

namespace CS
{
class RenderTargetDescriptor;

class RenderTarget final : public IGraphicsResource
{
public:
    friend class GraphicsAPI;
    ~RenderTarget() override;

    void SetColorAttachment(Texture texture);
    size_t GetColorAttachment() const;

    void SetDepthStencilAttachment(Texture texture);
    size_t GetDepthStencilAttachment() const;

    void SetSize(Size2u size);
    const Size2u& GetSize() const;

private:
    RenderTarget(RenderTargetDescriptor* descriptor);
};

} // namespace CS
