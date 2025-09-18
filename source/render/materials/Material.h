#pragma once
#include "MaterialInstance.h"
#include "graphics/ShaderStage.h"
#include "graphics/VertexInputFormat.h"
#include <vector>

namespace CS
{
class Shader;
class MaterialCompiler;
class ShaderBindingProperty;
class ShaderBindingTexture;

class Material : public std::enable_shared_from_this<Material>
{
public:
    class Builder
    {
    public:
        Builder& Begin();
        Builder& AddInputAttribute(uint32_t location, VertexInputFormat format);
        Builder& AddShader(std::shared_ptr<Shader> shader);

        std::shared_ptr<Material> End();

    private:
        uint32_t insertUniform(uint32_t binding, uint32_t offset, const ShaderBindingProperty& property);
        void insertUniformTexture(uint32_t binding, const ShaderBindingTexture& texture);

    private:
        static uint16_t ID;
        MaterialInstance::Uniforms m_uniforms;
        MaterialInstance::Textures m_textures;
        std::shared_ptr<Material> m_material;
    };
    friend class Builder;

    template <typename T>
    [[nodiscard]] bool SetUniformValue(std::string_view name, T&& value)
    {
        return m_defaultInstance->SetUniformValue(name, std::move(value));
    }

    std::shared_ptr<MaterialInstance> CreateInstance();

    void Compile(MaterialCompiler& compiler) const;
    uint16_t GetID() const { return m_id; }

    const MaterialInstance& GetDefaultInstance() const { return *m_defaultInstance; }

private:
    Material(/* args */) = default;
    void createDefaultInstance(MaterialInstance::Uniforms uniforms, MaterialInstance::Textures textures = {});

private:
    uint16_t m_id{0u};
    uint32_t m_instanceID{0u};

    std::map<uint32_t, VertexInputFormat> m_attributes;
    std::vector<std::shared_ptr<Shader>> m_shaders;
    std::unique_ptr<MaterialInstance> m_defaultInstance;
};

} // namespace CS
