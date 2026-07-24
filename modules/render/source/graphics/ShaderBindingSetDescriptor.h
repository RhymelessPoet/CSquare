#pragma once
#include "IGraphicsResourceDescriptor.h"
#include "ShaderBinding.h"
#include "TypeTraits.h"
#include <variant>
#include <vector>

namespace CS
{
class GraphicsBufferDescriptor;
class TextureDescriptor;
class SamplerDescriptor;

class ShaderBindingSetLayoutDescriptor final : public IGraphicsResourceDescriptor
{
public:
    ShaderBindingSetLayoutDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI);
    ~ShaderBindingSetLayoutDescriptor() = default;

    virtual bool IsBuild() const override;
    virtual void Destroy() override;
    virtual bool IsDirty() const override;

    void AddBinding(const ShaderBinding& binding);
    const std::vector<ShaderBinding>& GetBindings() const { return m_bindings; }

    size_t GetBindingSize(size_t binding) const;

private:
    virtual bool build() override;

private:
    std::vector<ShaderBinding> m_bindings;
};

class ShaderBindingSetDescriptor final : public IGraphicsResourceDescriptor
{
public:
    struct UniformBufferBinding
    {
        GraphicsBufferDescriptor* buffer{nullptr};
        size_t offset{0u};
        size_t range{0u};
    };

    struct SampledTextureBinding
    {
        TextureDescriptor* texture{nullptr};
        SamplerDescriptor* sampler{nullptr};
    };

    using BindingInfo = std::variant<UniformBufferBinding, SampledTextureBinding>;

    ShaderBindingSetDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI, size_t layoutResourceID);
    ~ShaderBindingSetDescriptor() = default;

    virtual bool IsBuild() const override;
    virtual void Destroy() override;
    virtual bool IsDirty() const override;

    bool BindUniformBuffer(size_t binding, size_t bufferID, size_t offset, size_t range);
    bool BindSampledTexture(size_t binding, size_t textureID, size_t samplerID);

    ShaderBindingSetLayoutDescriptor* GetLayout() const { return m_layout; }

    template <typename T>
    std::optional<T> GetBindingInfo(size_t binding) const
        requires type_traits::is_in_variant_v<T, ShaderBindingSetDescriptor::BindingInfo>
    {
        if (binding >= m_bindings.size()) {
            return std::nullopt;
        }
        auto& info = m_bindings[binding];
        if (std::holds_alternative<T>(info)) {
            return std::get<T>(info);
        }
        return std::nullopt;
    }

private:
    void setLayout(size_t layoutResourceID);
    virtual bool build() override;

private:
    ShaderBindingSetLayoutDescriptor* m_layout{nullptr};
    std::vector<BindingInfo> m_bindings;
};

} // namespace CS
