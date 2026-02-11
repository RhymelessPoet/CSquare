#pragma once
#include "model/TreeModel.h"
#include "scene/Scene.h"
#include "scene/SceneObject.h"
#include <unordered_map>

namespace CS
{
class Scene;
class SceneObject;
} // namespace CS

namespace CSEditor
{

template <>
struct TreeOperationStrategy<std::shared_ptr<CS::Scene>, std::shared_ptr<CS::SceneObject>>
{
    static std::shared_ptr<CS::SceneObject> GetParent(const std::shared_ptr<CS::SceneObject>& so)
    {
        return so->GetParent();
    }

    static std::shared_ptr<CS::SceneObject> GetChild(const std::shared_ptr<CS::SceneObject>& so, size_t index)
    {
        return so->GetChildren()[index];
    }

    static size_t GetChildCount(const std::shared_ptr<CS::SceneObject>& so) { return so->GetChildren().size(); }

    static bool IsValid(const std::shared_ptr<CS::SceneObject>& so) { return so != nullptr; }

    static std::shared_ptr<CS::SceneObject> GetRoot(const std::shared_ptr<CS::Scene>& scene)
    {
        return scene->GetRoot();
    }

    static std::shared_ptr<CS::SceneObject> CreateNode(const std::shared_ptr<CS::Scene>& scene,
                                                       const std::shared_ptr<CS::SceneObject>& parent)
    {
        return scene->CreateSceneObject(parent);
    }
};

template <>
struct TreeExtendStrategy<std::shared_ptr<CS::SceneObject>, bool, bool>
{
    using ExtendedPropertiesTuple = std::tuple<bool, bool>;

    std::vector<std::string_view> GetPropertyNames() const { return PropertyNames; }

    void Initialize(ExtendedPropertiesTuple& props) {}

    void Insert(const std::shared_ptr<CS::SceneObject>& so, const TreeNode* node) { Nodes.emplace(so.get(), node); }

    void Remove(const std::shared_ptr<CS::SceneObject>& so) { Nodes.erase(so.get()); }

    const TreeNode* Get(const std::shared_ptr<CS::SceneObject>& so) const
    {
        auto it = Nodes.find(so.get());
        return it != Nodes.end() ? it->second : nullptr;
    }

    std::any GetProperty(const std::shared_ptr<CS::SceneObject>& node,
                         const ExtendedPropertiesTuple& props,
                         std::string_view property) const
    {
        if (property == "name") {
            return std::any{node->GetName()};
        }
        if (property == "expand") {
            return std::any{std::get<0>(props)};
        }
        if (property == "active") {
            return std::any{node->IsActive()};
        }
        return std::any{};
    }

private:
    static inline std::unordered_map<CS::SceneObject*, const TreeNode*> Nodes;
    static inline std::vector<std::string_view> PropertyNames{"expand", "name", "active"};
};
using SceneTreeExtendStrategy = TreeExtendStrategy<std::shared_ptr<CS::SceneObject>, bool, bool>;

static inline TreeModel MakeSceneTreeModel(const std::shared_ptr<CS::Scene>& scene)
{
    return TreeModel(scene, std::shared_ptr<CS::SceneObject>{}, SceneTreeExtendStrategy{}, bool{}, bool{});
}

} // namespace CSEditor
