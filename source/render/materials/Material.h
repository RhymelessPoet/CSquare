#pragma once
#include "graphics/ShaderStage.h"
#include "graphics/VertexInputFormat.h"
#include <map>
#include <memory>
#include <vector>

namespace CS
{

class Shader;
class MaterialComputer;

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
        static size_t ID;
        std::shared_ptr<Material> m_material;
    };
    friend class Builder;

    void Compute(MaterialComputer& computer) const;
    size_t GetID() const { return m_id; }

private:
    Material(/* args */) = default;

private:
    size_t m_id{0u};
    std::map<uint32_t, VertexInputFormat> m_attributes;
    std::vector<std::shared_ptr<Shader>> m_shaders;
};

} // namespace CS
