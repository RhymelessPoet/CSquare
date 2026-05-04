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

void RenderTarget::SetDepthStencilAttachment(Texture texture)
{
    auto& renderTargetDescriptor = descriptor<RenderTargetDescriptor>();
    renderTargetDescriptor.SetDepthStencilAttachment(texture.GetID());
}

size_t RenderTarget::GetDepthStencilAttachment() const
{
    if (auto depthStencilAttachment = descriptor<RenderTargetDescriptor>().GetDepthStencilAttachment()) {
        return depthStencilAttachment->GetID();
    }
    return -1u;
}

void RenderTarget::SetDepthAttachment(Texture texture)
{
    auto& renderTargetDescriptor = descriptor<RenderTargetDescriptor>();
    renderTargetDescriptor.SetDepthAttachment(texture.GetID());
}

size_t RenderTarget::GetDepthAttachment() const
{
    if (auto depthAttachment = descriptor<RenderTargetDescriptor>().GetDepthAttachment()) {
        return depthAttachment->GetID();
    }
    return -1u;
}

void RenderTarget::SetSize(Size2u size)
{
    descriptor<RenderTargetDescriptor>().SetSize(size);
}

const Size2u& RenderTarget::GetSize() const
{
    return descriptor<RenderTargetDescriptor>().GetSize();
}

} // namespace CS
