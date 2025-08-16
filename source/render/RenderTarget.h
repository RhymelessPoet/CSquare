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

    void SetSize(Size2U size);
    const Size2U& GetSize() const;

private:
    RenderTarget(RenderTargetDescriptor* descriptor);
};

} // namespace CS
