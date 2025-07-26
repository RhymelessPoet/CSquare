#pragma once
#include "GraphicsResourceDescriptors.h"
#include "IGraphicsCommand.h"
#include "IGraphicsResourceDescriptor.h"


#include <queue>

namespace CS
{
class GraphicsCommandBufferDescriptor final : public IGraphicsResourceDescriptor
{
public:
    friend class GraphicsGLImpl;

    GraphicsCommandBufferDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI);

    virtual bool IsBuild() const override;
    virtual void Destroy() override;

    template <typename CommandType, typename... Args>
    void Push(Args&&... args)
    {
        m_buffer.push(std::make_unique<CommandType>(std::forward<Args>(args)...));
    }

private:
    std::queue<std::unique_ptr<IGraphicsCommand>> m_buffer;
};
} // namespace CS
