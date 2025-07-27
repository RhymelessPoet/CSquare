#pragma once
#include "GraphicsResourceDescriptors.h"
#include "IGraphicsResourceDescriptor.h"
#include <queue>

namespace CS
{
class IGraphicsCommand;
class GraphicsCommandBufferDescriptor final : public IGraphicsResourceDescriptor
{
public:
    friend class GraphicsGLImpl;

    GraphicsCommandBufferDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI);

    virtual bool IsBuild() const override;
    virtual void Destroy() override;

    void Clear();

    void Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI);

    template <typename CommandType, typename... Args>
    void Push(Args&&... args)
    {
        m_buffer.push(std::make_shared<CommandType>(std::forward<Args>(args)...));
    }

protected:
    virtual bool build() override;

private:
    std::queue<std::shared_ptr<IGraphicsCommand>> m_buffer;
};
} // namespace CS
