#pragma once
#include "ShaderStage.h"
#include <optional>
#include <string>

namespace CS
{

class ShaderBinding
{
public:
    enum class Type
    {
        UniformBuffer,
        StorageBuffer,
        SampledImage,
        StorageImage,
        Sampler,
        AccelerationStructure,
        Max
    };
    ShaderBinding(uint32_t binding, ShaderStageFlags stages, Type type);
    ShaderBinding(uint32_t binding, ShaderStage stage, Type type);
    ~ShaderBinding() = default;

    uint32_t GetBinding() const { return m_binding; }
    std::optional<uint32_t> GetSet() const { return m_set; }
    ShaderStageFlags GetShaderStages() const { return m_stages; }
    Type GetType() const { return m_type; }

private:
    uint32_t m_binding;
    std::optional<uint32_t> m_set;
    ShaderStageFlags m_stages;
    Type m_type{Type::Max};
    std::string m_name;
};

} // namespace CS
