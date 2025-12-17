#pragma once
#include "IAssetLoaderImpl.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include <optional>
#include <vector>

namespace CS
{
class Image;
class Material;
class Mesh;
class MaterialInstance;
class AssetNode;
class AssetScene;
class Scene;

class AssimpLoader : public IAssetLoaderImpl
{
public:
    AssimpLoader(/* args */);

    virtual std::shared_ptr<AssetScene> Load(const Path& path) override;

private:
    bool parseTextures(const aiScene* aiscene, std::shared_ptr<AssetScene> scene);
    bool parseMaterials(const aiScene* aiscene, std::shared_ptr<AssetScene> scene);
    bool parseMeshs(const aiScene* aiscene, std::shared_ptr<AssetScene> scene);
    bool parseCameras(const aiScene* aiscene, std::shared_ptr<AssetNode> root);
    bool parseNodes(const aiNode* node, std::shared_ptr<AssetScene> scene);
    void parseNode(const aiNode* node, std::shared_ptr<AssetScene> scene, std::shared_ptr<AssetNode> assetNode);

    std::shared_ptr<Mesh> parseMesh(const aiMesh* aimesh);
    std::shared_ptr<MaterialInstance> parsePBR(const aiMaterial* assimpMaterial);

    void printMaterialInfo(const aiMaterial* material);
};

} // namespace CS
