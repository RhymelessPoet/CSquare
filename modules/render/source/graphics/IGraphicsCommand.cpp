#include "IGraphicsCommand.h"

namespace CS
{

void IGraphicsCommand::buildGraphicsResource(IGraphicsResourceDescriptor* descriptor)
{
    if (!descriptor->IsBuild()) {
        descriptor->Build();
    }
}

} // namespace CS
