#include "ShaderBindingSet.h"
#include "ShaderBindingSetDescriptor.h"

namespace CS
{
ShaderBindingSet::ShaderBindingSet(ShaderBindingSetDescriptor* descriptor) : IGraphicsResource(descriptor) {}

bool ShaderBindingSet::BindUniformBuffer(size_t binding, UniformBuffer buffer, size_t offset, size_t range)
{
    return descriptor<ShaderBindingSetDescriptor>().BindUniformBuffer(binding, buffer.GetID(), offset, range);
}

} // namespace CS
