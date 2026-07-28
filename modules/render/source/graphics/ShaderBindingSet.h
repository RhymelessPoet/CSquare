#pragma once
#include "IGraphicsResource.h"
#include "Sampler.h"
#include "Texture.h"
#include "UniformBuffer.h"
#include "StorageBuffer.h"
#include "GraphicsResourceParameters.h"

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
    bool BindStorageBuffer(size_t binding, StorageBuffer buffer, size_t offset, size_t range);
    bool BindStorageTexture(size_t binding, Texture texture, StorageTextureAccess access, uint32_t mipLevel = 0);

    size_t GetBindingSize(size_t binding) const;

private:
    ShaderBindingSet(ShaderBindingSetDescriptor* descriptor);
};

} // namespace CS
