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

    void SetSize(Size2U size);

private:
    RenderTarget(RenderTargetDescriptor* descriptor);
};

} // namespace CS
