#include "AssetImporter.h"
#include "asset/AssetNode.h"
#include "asset/AssetScene.h"
#include "geometry/GeometryNode.h"
#include "scene/MeshRenderSystem.h"
#include "scene/MeshRenderer.h"
#include "scene/Scene.h"
#include "scene/SceneObject.h"
#include "scene/Transform.h"
#include "scene/TransformSystem.h"

namespace CS
{
AssetImporter::AssetImporter() : m_scene(std::make_shared<Scene>()) {}

AssetImporter::AssetImporter(std::shared_ptr<Scene> scene) : m_scene(scene) {}

std::shared_ptr<SceneObject> AssetImporter::Import(const std::shared_ptr<AssetNode>& assetNode)
{
    auto sceneObject = m_scene->CreateSceneObject();
    importNode(assetNode, sceneObject);
    return sceneObject;
}

std::shared_ptr<Scene> AssetImporter::Import(const std::shared_ptr<AssetScene>& assetScene)
{
    importNode(assetScene->GetRoot(), m_scene->GetRoot());
    return m_scene;
}

void AssetImporter::importNode(const std::shared_ptr<AssetNode>& assetNode, std::shared_ptr<SceneObject> sceneObject)
{
    sceneObject->SetName(assetNode->GetName());
    // Create Transform component
    auto transform = createTransform(sceneObject);
    transform->SetPosition(assetNode->GetPosition());
    transform->SetRotation(assetNode->GetRotation());
    transform->SetScale(assetNode->GetScale());

    // Create MeshRenderer components
    auto meshRenderer = createMeshRenderer(sceneObject);
    for (const auto& [mesh, material] : assetNode->GetMeshs()) {
        auto geometryNode = createGeometryNode(mesh, material);
        meshRenderer->AddGeometryNode(geometryNode);
    }

    // Recursively import child nodes
    const auto& children = assetNode->GetChildren();
    for (const auto& childAssetNode : children) {
        auto childSceneObject = m_scene->CreateSceneObject(sceneObject);
        sceneObject->AddChild(childSceneObject);
        importNode(childAssetNode, childSceneObject);
    }
}

MeshRenderer* AssetImporter::createMeshRenderer(std::shared_ptr<SceneObject> sceneObject)
{
    auto& meshRenderSystem = m_scene->GetSystem<MeshRenderSystem>();
    meshRenderSystem.CreateComponent<MeshRenderer>(sceneObject);
    return sceneObject->GetComponent<MeshRenderer>();
}

Transform* AssetImporter::createTransform(std::shared_ptr<SceneObject> sceneObject)
{
    auto& transformSystem = m_scene->GetSystem<TransformSystem>();
    transformSystem.CreateComponent<Transform>(sceneObject);
    return sceneObject->GetComponent<Transform>();
}

std::shared_ptr<GeometryNode> AssetImporter::createGeometryNode(std::shared_ptr<Mesh> mesh,
                                                                std::shared_ptr<MaterialInstance> material)
{
    auto node = std::make_shared<GeometryNode>(mesh, std::move(material));
    uint32_t location = 0;
    mesh->ForEachAttribute(
        [&](const Mesh::Attribute& attribute) { node->SetAttributeMap(attribute.name, location++); });
    return node;
}

} // namespace CS
