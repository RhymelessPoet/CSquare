#pragma once
#include "RenderTarget.h"
#include "base/Color.h"
#include <optional>

namespace CS
{

class GraphicsCommandBufferDescriptor;

class GraphicsCommandBuffer final : public IGraphicsResource
{
public:
    class GraphicsCommandBufferImpl;
    ~GraphicsCommandBuffer();

    GraphicsCommandBuffer& BeginPass(RenderTarget renderTarget);
    GraphicsCommandBuffer& Clear(std::optional<Color> clearColor);
    GraphicsCommandBuffer& EndPass();

private:
    GraphicsCommandBuffer(GraphicsCommandBufferDescriptor* descriptor);
};

} // namespace CS
