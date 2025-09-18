#include "ShaderBindingSet.h"
#include "ShaderBindingSetDescriptor.h"

namespace CS
{
ShaderBindingSet::ShaderBindingSet(ShaderBindingSetDescriptor* descriptor) : IGraphicsResource(descriptor) {}

bool ShaderBindingSet::BindUniformBuffer(size_t binding, UniformBuffer buffer, size_t offset, size_t range)
{
    return descriptor<ShaderBindingSetDescriptor>().BindUniformBuffer(binding, buffer.GetID(), offset, range);
}

bool ShaderBindingSet::BindSampledTexture(size_t binding, Texture texture, Sampler sampler)
{
    return descriptor<ShaderBindingSetDescriptor>().BindSampledTexture(binding, texture.GetID(), sampler.GetID());
}

size_t ShaderBindingSet::GetBindingSize(size_t binding) const
{
    return descriptor<ShaderBindingSetDescriptor>().GetLayout()->GetBindingSize(binding);
}

} // namespace CS
