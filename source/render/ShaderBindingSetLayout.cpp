#include "ShaderBindingSetLayout.h"
#include "ShaderBindingSetDescriptor.h"

namespace CS
{
ShaderBindingSetLayout::ShaderBindingSetLayout(ShaderBindingSetLayoutDescriptor* descriptor)
    : IGraphicsResource(descriptor)
{}

ShaderBindingSetLayout& ShaderBindingSetLayout::AddBinding(const ShaderBinding& binding)
{
    descriptor<ShaderBindingSetLayoutDescriptor>().AddBinding(binding);

    return *this;
}

const std::vector<ShaderBinding>& ShaderBindingSetLayout::GetBindings() const
{
    return descriptor<ShaderBindingSetLayoutDescriptor>().GetBindings();
}

} // namespace CS
