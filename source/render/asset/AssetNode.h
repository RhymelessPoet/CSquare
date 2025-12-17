#pragma once
#include "base/math/Matrix.h"
#include <memory>
#include <string>
#include <vector>

namespace CS
{

class MaterialInstance;
class Mesh;

class AssetNode
{
public:
    using Meshes = std::vector<std::pair<std::shared_ptr<Mesh>, std::shared_ptr<MaterialInstance>>>;
    explicit AssetNode(std::shared_ptr<AssetNode> parent = nullptr);

    void SetName(std::string_view name) { m_name = name; }
    const std::string_view GetName() const { return m_name; }

    void SetParent(std::shared_ptr<AssetNode> parent) { m_parent = parent; }
    std::shared_ptr<AssetNode> GetParent() const { return m_parent.lock(); }

    void AddChild(std::shared_ptr<AssetNode> child) { m_children.push_back(child); }
    const std::vector<std::shared_ptr<AssetNode>>& GetChildren() const { return m_children; }

    void SetPosition(const Vector3f& position) { m_position = position; }
    const Vector3f& GetPosition() const { return m_position; }
    void SetRotation(const Vector3f& rotation) { m_rotation = rotation; }
    const Vector3f& GetRotation() const { return m_rotation; }
    void SetScale(const Vector3f& scale) { m_scale = scale; }
    const Vector3f& GetScale() const { return m_scale; }

    void AddMesh(std::shared_ptr<Mesh> mesh, std::shared_ptr<MaterialInstance> matereial);
    const Meshes& GetMeshs() const { return m_meshs; }

private:
    std::string m_name;
    std::weak_ptr<AssetNode> m_parent;
    std::vector<std::shared_ptr<AssetNode>> m_children;

    Vector3f m_position;
    Vector3f m_rotation;
    Vector3f m_scale;

    Meshes m_meshs;
};

} // namespace CS
