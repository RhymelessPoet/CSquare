#pragma once
#include <memory>

namespace CS
{

class AssetScene;
class AssetNode;
class SceneObject;
class Scene;
class MeshRenderer;
class Transform;
class Mesh;
class Material;
class GeometryNode;
class MaterialInstance;

class AssetImporter
{
public:
    AssetImporter();
    AssetImporter(std::shared_ptr<Scene> scene);

    std::shared_ptr<SceneObject> Import(const std::shared_ptr<AssetNode>& assetNode);
    std::shared_ptr<Scene> Import(const std::shared_ptr<AssetScene>& assetScene);

private:
    void importNode(const std::shared_ptr<AssetNode>& assetNode, std::shared_ptr<SceneObject> sceneObject);
    MeshRenderer* createMeshRenderer(std::shared_ptr<SceneObject> sceneObject);
    Transform* createTransform(std::shared_ptr<SceneObject> sceneObject);
    std::shared_ptr<GeometryNode> createGeometryNode(std::shared_ptr<Mesh> mesh,
                                                     std::shared_ptr<MaterialInstance> material);

private:
    std::shared_ptr<Scene> m_scene;
};

} // namespace CS
