#pragma once
#include "graphics/GraphicsGLImpl.h"

namespace CS
{
class IGraphicsCommand
{
public:
    IGraphicsCommand(/* args */) = default;
    virtual ~IGraphicsCommand() {}

    virtual bool Execute(std::shared_ptr<GraphicsGLImpl>& graphicsAPI) = 0;

protected:
    void buildGraphicsResource(IGraphicsResourceDescriptor* descriptor);
};

} // namespace CS
