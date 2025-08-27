#pragma once
#include "IGraphicsResource.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"

namespace CS
{
class GraphicsInputAssemblyDescriptor;
class VertexInputLayout;

class GraphicsInputAssembly : public IGraphicsResource
{
public:
    friend class GraphicsAPI;

    GraphicsInputAssembly(/* args */) = default;
    ~GraphicsInputAssembly() = default;

    void SetVertexInputLayout(std::unique_ptr<VertexInputLayout> inputLayout);

    void SetVertexInput(uint32_t binding, VertexBuffer buffer, uint32_t offset);
    void SetIndexBuffer(IndexBuffer indexBuffer, uint16_t);
    void SetIndexBuffer(IndexBuffer indexBuffer, uint32_t);

private:
    GraphicsInputAssembly(GraphicsInputAssemblyDescriptor* descriptor);
};

} // namespace CS
