#pragma once
#include "model/TreeModel.h"
#include "scene/CameraComponent.h"
#include "scene/LightComponent.h"
#include "scene/MeshRenderer.h"
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
struct TreeExtendStrategy<std::shared_ptr<CS::SceneObject>, bool, std::string>
{
    using ExtendedPropertiesTuple = std::tuple<bool, std::string>;

    std::vector<std::string_view> GetPropertyNames() const { return PropertyNames; }

    void Initialize(const std::shared_ptr<CS::SceneObject>& node, ExtendedPropertiesTuple& props)
    {
        std::get<0>(props) = false;
        if (node->GetComponent<CS::LightComponent>() != nullptr) {
            std::get<1>(props) = "light";
        } else if (node->GetComponent<CS::CameraComponent>() != nullptr) {
            std::get<1>(props) = "camera";
        } else if (node->GetComponent<CS::MeshRenderer>() != nullptr) {
            std::get<1>(props) = "model";
        } else if (!node->GetChildren().empty()) {
            std::get<1>(props) = "group";
        } else {
            std::get<1>(props) = "invalid";
        }
    }

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
        if (property == "object_type") {
            return std::any(std::get<1>(props));
        }
        return std::any{};
    }

    bool SetProperty(std::shared_ptr<CS::SceneObject>& node,
                     ExtendedPropertiesTuple& props,
                     std::string_view property,
                     const std::any& value)
    {
        return setProperty(node, property, value) || setProperty(props, property, value);
    }

private:
    bool setProperty(std::shared_ptr<CS::SceneObject>& node, std::string_view property, const std::any& value)
    {
        if (property == "name") {
            if (value.type() == typeid(std::string)) {
                node->SetName(std::any_cast<std::string>(value));
                return true;
            }
        } else if (property == "active") {
            if (value.type() == typeid(bool)) {
                node->SetActive(std::any_cast<bool>(value));
                return true;
            }
        }
        return false;
    }

    bool setProperty(ExtendedPropertiesTuple& props, std::string_view property, const std::any& value)
    {
        if (property == "expand") {
            if (value.type() == typeid(bool)) {
                std::get<0>(props) = std::any_cast<bool>(value);
                return true;
            }
        }
        return false;
    }

private:
    static inline std::unordered_map<CS::SceneObject*, const TreeNode*> Nodes;
    static inline std::vector<std::string_view> PropertyNames{"expand", "name", "active", "object_type"};
};
using SceneTreeExtendStrategy = TreeExtendStrategy<std::shared_ptr<CS::SceneObject>, bool, std::string>;

static inline TreeModel MakeSceneTreeModel(const std::shared_ptr<CS::Scene>& scene)
{
    return TreeModel(scene, std::shared_ptr<CS::SceneObject>{}, SceneTreeExtendStrategy{}, bool{}, std::string{});
}

} // namespace CSEditor
