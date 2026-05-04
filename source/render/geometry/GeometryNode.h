#pragma once
#include "Mesh.h"
#include "scene/ViewType.h"
#include <map>
#include <memory>
#include <string>

namespace CS
{
class Mesh;
class MaterialInstance;
class VertexInputLayout;

class GeometryNode
{
public:
    GeometryNode(std::shared_ptr<Mesh> mesh, std::shared_ptr<MaterialInstance> material);

    void SetAttributeMap(std::string_view attribute, uint32_t location);
    bool RemoveAttributeMap(std::string_view attribute);

    std::shared_ptr<VertexInputLayout> GetVertexInputLayout();

    std::shared_ptr<Mesh> GetMesh() const { return m_mesh; }
    std::shared_ptr<MaterialInstance> GetMaterial(EViewType viewType = EViewType{}) const;

private:
    void makeVertexInputLayout(const Mesh::Attribute& attribute,
                               std::vector<std::pair<uint8_t, uint32_t>>& bindings) const;

private:
    std::shared_ptr<Mesh> m_mesh;
    std::shared_ptr<MaterialInstance> m_material;
    std::map<std::string_view, uint32_t> m_attributeMap;
    std::shared_ptr<VertexInputLayout> m_vertexInputLayout;
};

} // namespace CS
