#include "AssetImporter.h"
#include "asset/AssetNode.h"
#include "asset/AssetScene.h"
#include "base/Logger.h"
#include "geometry/GeometryNode.h"
#include "materials/MaterialInstance.h"
#include "scene/MeshRenderSystem.h"
#include "scene/MeshRenderer.h"
#include "scene/Scene.h"
#include "scene/SceneObject.h"
#include "scene/SceneObjectComposer.h"
#include "scene/Transform.h"
#include "scene/TransformSystem.h"
#include <chrono>

namespace CS
{
AssetImporter::AssetImporter(std::shared_ptr<SceneObjectComposer> composer)
    : m_scene(std::make_shared<Scene>(std::move(composer)))
{}

AssetImporter::AssetImporter(std::shared_ptr<Scene> scene) : m_scene(std::move(scene)) {}

std::shared_ptr<SceneObject> AssetImporter::Import(const std::shared_ptr<AssetNode>& assetNode)
{
    auto t0 = std::chrono::steady_clock::now();
    auto sceneObject = m_scene->CreateSceneObject();
    importNode(assetNode, sceneObject);
    CS::LogPerf(::CS::BuiltInChannels::Asset(),
                CS::Fmt("AssetImporter::Import node '{}' completed in {:.2f}ms", assetNode->GetName(),
                        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count()));
    return sceneObject;
}

std::shared_ptr<Scene> AssetImporter::Import(const std::shared_ptr<AssetScene>& assetScene)
{
    auto t0 = std::chrono::steady_clock::now();
    importNode(assetScene->GetRoot(), m_scene->GetRoot());
    CS::LogPerf(::CS::BuiltInChannels::Asset(),
                CS::Fmt("AssetImporter::Import scene total: {:.2f}ms",
                        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count()));
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
        importNode(childAssetNode, childSceneObject);
    }
}

MeshRenderer* AssetImporter::createMeshRenderer(std::shared_ptr<SceneObject> sceneObject)
{
    auto composer = m_scene->GetComposer();
    if (!composer->AddComponent<MeshRenderer>(sceneObject)) {
        return nullptr;
    }
    return sceneObject->GetComponent<MeshRenderer>();
}

Transform* AssetImporter::createTransform(std::shared_ptr<SceneObject> sceneObject)
{
    auto composer = m_scene->GetComposer();
    if (!composer->AddComponent<Transform>(sceneObject)) {
        return nullptr;
    }
    return sceneObject->GetComponent<Transform>();
}

std::shared_ptr<GeometryNode> AssetImporter::createGeometryNode(std::shared_ptr<Mesh> mesh,
                                                                std::shared_ptr<MaterialInstance> material)
{
    auto node = std::make_shared<GeometryNode>(mesh, material->Clone());
    uint32_t location = 0;
    mesh->ForEachAttribute(
        [&](const Mesh::Attribute& attribute) { node->SetAttributeMap(attribute.name, location++); });
    return node;
}

} // namespace CS
