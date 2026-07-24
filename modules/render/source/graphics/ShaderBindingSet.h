#pragma once
#include "IGraphicsResource.h"
#include "Sampler.h"
#include "Texture.h"
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
    bool BindSampledTexture(size_t binding, Texture texture, Sampler sampler);

    size_t GetBindingSize(size_t binding) const;

private:
    ShaderBindingSet(ShaderBindingSetDescriptor* descriptor);
};

} // namespace CS
