#include "GeometryNode.h"
#include "graphics/VertexInputLayout.h"
#include "materials/MaterialInstance.h"

namespace CS
{
GeometryNode::GeometryNode(std::shared_ptr<Mesh> mesh, std::shared_ptr<MaterialInstance> material)
    : m_mesh(std::move(mesh)), m_material(material)
{}

void GeometryNode::SetAttributeMap(std::string_view attribute, uint32_t location)
{
    m_attributeMap[attribute] = location;
}

bool GeometryNode::RemoveAttributeMap(std::string_view attribute)
{
    return m_attributeMap.erase(attribute) != 0u;
}

std::shared_ptr<VertexInputLayout> GeometryNode::GetVertexInputLayout()
{
    if (m_vertexInputLayout != nullptr) {
        return m_vertexInputLayout;
    }
    m_vertexInputLayout = std::make_shared<VertexInputLayout>();
    std::vector<std::pair<uint8_t, uint32_t>> bindings;
    m_mesh->ForEachAttribute([&](const Mesh::Attribute& attribute) { makeVertexInputLayout(attribute, bindings); });

    return m_vertexInputLayout;
}

std::shared_ptr<MaterialInstance> GeometryNode::GetMaterial(EViewType viewType) const
{
    if (viewType.IsUndefined() || m_material->GetViewType() == viewType) {
        return m_material;
    }
    return m_material->GetRequisiteMaterial(viewType);
}

void GeometryNode::makeVertexInputLayout(const Mesh::Attribute& attribute,
                                         std::vector<std::pair<uint8_t, uint32_t>>& bindings) const
{
    auto vertexCount = m_mesh->GetVertexCount();

    const auto& [name, offset, end, bufferID, componentCount, type] = attribute;
    auto componentByteSize = static_cast<uint32_t>(ByteSize(type)) * componentCount;
    auto stride = (end - offset - componentByteSize) / (vertexCount - 1u);
    uint32_t bindingNum = 0u;
    uint32_t componentOffset = 0u;
    if (stride == componentByteSize) {
        // tightly packed
        bindings.emplace_back(bufferID, offset - componentByteSize);
        bindingNum = bindings.size() - 1u;
        m_vertexInputLayout->SetBinding(bindingNum, VertexInputBinding{stride, VertexInputRate::PerVertex});
    } else {
        // interleaved
        componentOffset = offset;
        auto it = std::find_if(bindings.begin(), bindings.end(),
                               [bufferID](const auto& binding) { return binding.first == bufferID; });
        bindingNum = std::distance(bindings.begin(), it);

        if (it == bindings.end()) {
            bindings.emplace_back(bufferID, 0u);
            m_vertexInputLayout->SetBinding(bindingNum, VertexInputBinding{stride, VertexInputRate::PerVertex});
        }
    }

    auto vertexFormat = VertexInputFormatFrom(type, componentCount);
    auto itr = m_attributeMap.find(std::string_view(name));
    if (itr != m_attributeMap.end()) {
        m_vertexInputLayout->SetAttribute(itr->second, VertexInputAttribute{bindingNum, vertexFormat, componentOffset});
    }
}

} // namespace CS
