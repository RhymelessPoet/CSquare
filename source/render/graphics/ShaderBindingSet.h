#pragma once
#include "IGraphicsResource.h"
#include "UniformBuffer.h"

namespace CS
{
class ShaderBindingSetDescriptor;

class ShaderBindingSet : public IGraphicsResource
{
public:
    friend class GraphicsAPI;
    ShaderBindingSet() = default;
    ~ShaderBindingSet() = default;

    bool BindUniformBuffer(size_t binding, UniformBuffer buffer, size_t offset, size_t range);

private:
    ShaderBindingSet(ShaderBindingSetDescriptor* descriptor);
};

} // namespace CS
