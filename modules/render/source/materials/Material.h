#pragma once
#include "IMaterialConfiguration.h"
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
        Builder& Begin(std::unique_ptr<IMaterialConfiguration> configuration);
        Builder& AddInputAttribute(uint32_t location, VertexInputFormat format);
        Builder& AddShader(std::shared_ptr<Shader> shader);
        Builder& AddRequisiteMaterial(std::shared_ptr<Material> material);
        Builder& Connect(const SlotID& from, uint16_t toSlotIndex);
        Builder& Bind(uint16_t slotIndex, uint32_t binding);

        std::shared_ptr<Material> End();

    private:
        void insertUniform(uint32_t binding, const ShaderBindingProperty& property);
        void insertUniformTexture(uint32_t binding, const ShaderBindingTexture& texture);

        bool connectFrom(const SlotID& from, const SlotID& to);
        bool connectTo(const SlotID& from, const SlotID& to);
        void disconnectFrom(const SlotID& id);

    private:
        static uint16_t ID;
        MaterialInstance::Uniforms m_uniforms;
        MaterialInstance::Textures m_textures;
        std::shared_ptr<Material> m_material;
        std::map<SlotID, uint16_t> m_connects;
        std::map<uint16_t, uint32_t> m_bindings;
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
    MaterialInstance& GetDefaultInstance() { return *m_defaultInstance; }

    IMaterialConfiguration& GetConfiguration() { return *m_configuration; }

    std::span<std::shared_ptr<Material>> GetRequisiteMaterials() { return m_requisiteMaterials; }

private:
    Material(/* args */) = default;
    void createDefaultInstance(MaterialInstance::Uniforms uniforms, MaterialInstance::Textures textures = {});

private:
    uint16_t m_id{0u};
    uint32_t m_instanceID{0u};

    std::unique_ptr<IMaterialConfiguration> m_configuration;
    std::map<uint32_t, VertexInputFormat> m_attributes;
    std::vector<std::shared_ptr<Shader>> m_shaders;
    std::unique_ptr<MaterialInstance> m_defaultInstance;

    std::vector<std::shared_ptr<Material>> m_requisiteMaterials;
};

} // namespace CS
