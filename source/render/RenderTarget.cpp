#include "RenderTarget.h"
#include "GraphicsResourceDescriptors.h"

namespace CS
{
RenderTarget::RenderTarget(RenderTargetDescriptor* descriptor) : IGraphicsResource(descriptor) {}

RenderTarget::~RenderTarget() {}

void RenderTarget::SetColorAttachment(Texture texture)
{
    auto& renderTargetDescriptor = descriptor<RenderTargetDescriptor>();
    renderTargetDescriptor.SetColorAttachment(texture.GetID());
}

size_t RenderTarget::GetColorAttachment() const
{
    return descriptor<RenderTargetDescriptor>().GetColorAttachment()->GetID();
}

void RenderTarget::SetSize(Size2U size)
{
    descriptor<RenderTargetDescriptor>().SetSize(size);
}

} // namespace CS
