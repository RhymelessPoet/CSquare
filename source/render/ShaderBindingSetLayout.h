#pragma once
#include "IGraphicsResource.h"
#include "ShaderBinding.h"
#include <vector>

namespace CS
{

class ShaderBindingSetLayoutDescriptor;

class ShaderBindingSetLayout : public IGraphicsResource
{
public:
    friend class GraphicsAPI;
    ShaderBindingSetLayout() = default;
    ~ShaderBindingSetLayout() = default;

    ShaderBindingSetLayout& AddBinding(const ShaderBinding& binding);
    const std::vector<ShaderBinding>& GetBindings() const;

private:
    ShaderBindingSetLayout(ShaderBindingSetLayoutDescriptor* descriptor);
};

} // namespace CS
