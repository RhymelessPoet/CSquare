#include "GraphicsCommandBufferDescriptor.h"
#include "IGraphicsCommand.h"

namespace CS
{

GraphicsCommandBufferDescriptor::GraphicsCommandBufferDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI)
    : IGraphicsResourceDescriptor(id, graphicsAPI)
{}

bool GraphicsCommandBufferDescriptor::IsBuild() const
{
    return true;
}

void GraphicsCommandBufferDescriptor::Destroy() {}

void GraphicsCommandBufferDescriptor::Clear()
{
    m_buffer = std::queue<std::shared_ptr<IGraphicsCommand>>();
}

void GraphicsCommandBufferDescriptor::Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI)
{
    while (!m_buffer.empty()) {
        auto command = std::move(m_buffer.front());
        m_buffer.pop();
        command->Execute(graphicsAPI);
    }
}
bool GraphicsCommandBufferDescriptor::build()
{
    return true;
}
} // namespace CS
