#include "AssimpLoader.h"
#include "asset/AssetNode.h"
#include "asset/AssetScene.h"
#include "asset/BuiltInMaterials.h"
#include "base/math/Math.h"
#include "geometry/Mesh.h"
#include "materials/Material.h"
#include <cassert>
#include <iostream>

namespace CS
{
template <typename T>
static T GetProperty(const aiMaterial* material, const char* key, unsigned int type, unsigned int idx)
{
    T value;
    auto ret = material->Get(key, type, idx, value);
    if (ret != aiReturn_SUCCESS) {
        // TODO: log error
    }
    return value;
}

AssimpLoader::AssimpLoader() {}

std::shared_ptr<AssetScene> AssimpLoader::Load(const Path& path)
{
    Assimp::Importer importer;

    auto postprocessFlags =
        aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType;

    const aiScene* aiscene = importer.ReadFile(path.string(), postprocessFlags);

    if (aiscene == nullptr) {
        std::cerr << "Assimp error: " + std::string(importer.GetErrorString()) << "\n";
        return nullptr;
    }

    if (aiscene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        std::cerr << "Assimp error: " + std::string(importer.GetErrorString()) << "\n";
    }

    std::vector<std::shared_ptr<Image>> textures;
    std::vector<std::shared_ptr<MaterialInstance>> materials;
    std::vector<std::shared_ptr<Mesh>> meshs;
    std::vector<uint32_t> meshMaterialIndices;

    auto scene = std::make_shared<AssetScene>();

    bool noError = true;
    noError = noError && parseTextures(aiscene, scene);
    noError = noError && parseMaterials(aiscene, scene);
    noError = noError && parseMeshs(aiscene, scene);
    noError = noError && parseNodes(aiscene->mRootNode, scene);

    return scene;
}

bool AssimpLoader::parseCameras(const aiScene* scene, std::shared_ptr<AssetNode> root)
{
    return true;
}

bool AssimpLoader::parseTextures(const aiScene* aiscene, std::shared_ptr<AssetScene> scene)
{
    for (uint32_t index = 0u; index < aiscene->mNumTextures; ++index) {
        auto texture = aiscene->mTextures[index];
        if (texture == nullptr) {
            // TODO: log error
            continue;
        }
        auto name = texture->mFilename.C_Str();
        texture->achFormatHint;
    }
    return true;
}

bool AssimpLoader::parseMaterials(const aiScene* aiscene, std::shared_ptr<AssetScene> scene)
{
    for (uint32_t index = 0u; index < aiscene->mNumMaterials; ++index) {
        auto aimaterial = aiscene->mMaterials[index];
        if (aimaterial == nullptr) {
            // TODO: log error
            continue;
        }
        printMaterialInfo(aimaterial);
        const auto mode = GetProperty<int>(aimaterial, AI_MATKEY_SHADING_MODEL);
        if (mode == aiShadingMode_PBR_BRDF) {
            scene->AddMaterial(parsePBR(aimaterial));
        }
        std::cerr << "\n";
    }

    return true;
}

bool AssimpLoader::parseMeshs(const aiScene* aiscene, std::shared_ptr<AssetScene> scene)
{
    for (uint32_t index = 0u; index < aiscene->mNumMeshes; ++index) {
        auto aimesh = aiscene->mMeshes[index];
        scene->AddMesh(parseMesh(aimesh), aimesh->mMaterialIndex);
    }
    return true;
}

std::shared_ptr<Mesh> AssimpLoader::parseMesh(const aiMesh* aimesh)
{
    auto builder = Mesh::Builder();
    builder.SetName(aimesh->mName.C_Str());

    Buffer vertexBuffer;
    uint32_t offset = vertexBuffer.GetByteSize();
    uint32_t end = vertexBuffer.GetByteSize();

    if (aimesh->mVertices != nullptr) {
        vertexBuffer.PushBack(aimesh->mVertices, aimesh->mNumVertices);
        end = vertexBuffer.GetByteSize();
        builder.AddAttribute({"_position", offset, end, 0u, 3u, DataType::Float32});
    }

    if (aimesh->mNormals != nullptr) {
        offset = end;
        vertexBuffer.PushBack(aimesh->mNormals, aimesh->mNumVertices);
        end = vertexBuffer.GetByteSize();
        builder.AddAttribute({"_normal", offset, end, 0u, 3u, DataType::Float32});
    }

    if (aimesh->mTangents != nullptr) {
        offset = end;
        vertexBuffer.PushBack(aimesh->mTangents, aimesh->mNumVertices);
        end = vertexBuffer.GetByteSize();
        builder.AddAttribute({"_tangent", offset, end, 0u, 3u, DataType::Float32});
    }

    if (aimesh->mBitangents != nullptr) {
        offset = end;
        vertexBuffer.PushBack(aimesh->mBitangents, aimesh->mNumVertices);
        end = vertexBuffer.GetByteSize();
        builder.AddAttribute({"_bitangent", offset, end, 0u, 3u, DataType::Float32});
    }

    for (uint32_t texCoordIndex = 0u; texCoordIndex < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++texCoordIndex) {
        if (aimesh->mTextureCoords[texCoordIndex] == nullptr) {
            break;
        }
        auto componentCount = static_cast<uint8_t>(aimesh->mNumUVComponents[texCoordIndex]);
        auto attributeName = "_texcoord" + std::to_string(texCoordIndex);
        offset = end;
        for (uint32_t i = 0u; i < aimesh->mNumVertices; ++i) {
            // Assimp always use 3 components for texture coordinates
            aiVector3D uvw = aimesh->mTextureCoords[texCoordIndex][i];
            aiVector2D uv{uvw.x, uvw.y};
            vertexBuffer.PushBack(&uv, 1u);
        }
        end = vertexBuffer.GetByteSize();
        builder.AddAttribute({attributeName, offset, end, 0u, componentCount, DataType::Float32});
    }

    for (uint32_t colorIndex = 0u; colorIndex < AI_MAX_NUMBER_OF_COLOR_SETS; ++colorIndex) {
        if (aimesh->mColors[colorIndex] == nullptr) {
            break;
        }
        auto attributeName = "_color" + std::to_string(colorIndex);
        offset = end;
        vertexBuffer.PushBack(aimesh->mColors[colorIndex], aimesh->mNumVertices);
        end = vertexBuffer.GetByteSize();
        builder.AddAttribute({attributeName, offset, end, 0u, 4u, DataType::Float32});
    }
    builder.AddVertexBuffer(vertexBuffer);

    std::vector<uint32_t> indices;
    indices.reserve(aimesh->mFaces[0].mNumIndices * aimesh->mNumFaces);

    for (uint32_t faceIndex = 0u; faceIndex < aimesh->mNumFaces; ++faceIndex) {
        auto aiface = aimesh->mFaces[faceIndex];
        for (uint32_t index = 0u; index < aiface.mNumIndices; ++index) {
            indices.push_back(aiface.mIndices[index]);
        }
    }
    builder.SetIndices(Buffer(indices), DataType::UInt32).SetVertexCount(aimesh->mNumVertices);

    if (!aimesh->mAABB.mMin.Equal(aimesh->mAABB.mMax)) {
        builder.SetAABB({{aimesh->mAABB.mMin.x, aimesh->mAABB.mMin.y, aimesh->mAABB.mMin.z},
                         {aimesh->mAABB.mMax.x, aimesh->mAABB.mMax.y, aimesh->mAABB.mMax.z}});
    }

    return builder.Build();
}

bool AssimpLoader::parseNodes(const aiNode* node, std::shared_ptr<AssetScene> scene)
{
    if (node == nullptr) {
        return false;
    }
    auto root = scene->GetRoot();
    parseNode(node, scene, root);

    return true;
}

void AssimpLoader::parseNode(const aiNode* node,
                             std::shared_ptr<AssetScene> scene,
                             std::shared_ptr<AssetNode> assetNode)
{

    assetNode->SetName(node->mName.C_Str());

    auto transform = node->mTransformation;
    aiVector3D position, scale;
    aiQuaternion rotation;
    transform.Decompose(scale, rotation, position);

    assetNode->SetPosition({position.x, position.y, position.z});
    assetNode->SetRotation(Math::ToEulerAnglesXYZ(Quaternion{rotation.x, rotation.y, rotation.z, rotation.w}));
    assetNode->SetScale({scale.x, scale.y, scale.z});

    for (uint32_t index = 0u; index < node->mNumMeshes; ++index) {
        auto meshIndex = node->mMeshes[index];
        auto mesh = scene->GetMesh(meshIndex);
        auto materialIndex = scene->GetMaterialIndex(meshIndex);

        if (materialIndex.has_value()) {
            auto material = scene->GetMaterial(materialIndex.value());
            assetNode->AddMesh(mesh, material);
        }
    }

    for (uint32_t index = 0u; index < node->mNumChildren; ++index) {
        auto childNode = node->mChildren[index];
        auto childAssetNode = std::make_shared<AssetNode>(assetNode);
        assetNode->AddChild(childAssetNode);
        parseNode(childNode, scene, childAssetNode);
    }
}

std::shared_ptr<MaterialInstance> AssimpLoader::parsePBR(const aiMaterial* aimaterial)
{
    auto material = BuiltInMaterials::Instance().GetPBRMaterial();
    auto materialInstance = material->CreateInstance();

    materialInstance->SetName(aimaterial->GetName().C_Str());

    aiColor4D color;
    bool noError = true;
    if (aimaterial->Get(AI_MATKEY_BASE_COLOR, color) == AI_SUCCESS) {
        noError =
            noError && materialInstance->SetUniformValue("base_color", Vector4f{color.r, color.g, color.b, color.a});
    }

    if (aimaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
        noError =
            noError && materialInstance->SetUniformValue("diffuse_color", Vector4f{color.r, color.g, color.b, color.a});
    }

    if (aimaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS) {
        noError = noError &&
                  materialInstance->SetUniformValue("emission_color", Vector4f{color.r, color.g, color.b, color.a});
    }

    float metallic = 0.0f;
    if (aimaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
        noError = noError && materialInstance->SetUniformValue("metallic", metallic);
    }

    float roughness = 1.0f;
    if (aimaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS) {
        noError = noError && materialInstance->SetUniformValue("roughness", roughness);
    }
    float shininess = 0.0f;
    if (aimaterial->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
        noError = noError && materialInstance->SetUniformValue("shininess", shininess);
    }

    return materialInstance;
}

void AssimpLoader::printMaterialInfo(const aiMaterial* material)
{
    std::cerr << "------------------ Parse Material: " << material->GetName().C_Str() << " ------------------\n";
    for (uint32_t index = 0u; index < material->mNumProperties; ++index) {
        auto aiproperty = material->mProperties[index];
        std::cerr << aiproperty->mKey.C_Str() << ": ";
        if (aiproperty->mType == aiPTI_Float) {
            auto count = aiproperty->mDataLength / sizeof(float);
            auto data = reinterpret_cast<float*>(aiproperty->mData);
            for (uint32_t i = 0u; i < count; ++i) {
                std::cerr << data[i] << " ";
            }
        } else if (aiproperty->mType == aiPTI_String) {
            aiString str;
            memcpy(&str, aiproperty->mData, sizeof(aiString));
            std::cerr << str.C_Str();
        } else if (aiproperty->mType == aiPTI_Integer) {
            auto count = aiproperty->mDataLength / sizeof(int);
            auto data = reinterpret_cast<int*>(aiproperty->mData);
            for (uint32_t i = 0u; i < count; ++i) {
                std::cerr << data[i] << " ";
            }
        } else if (aiproperty->mType == aiPTI_Double) {
            auto count = aiproperty->mDataLength / sizeof(double);
            auto data = reinterpret_cast<double*>(aiproperty->mData);
            for (uint32_t i = 0u; i < count; ++i) {
                std::cerr << data[i] << " ";
            }
        } else if (aiproperty->mType == aiPTI_Buffer) {
            if (strcmp(aiproperty->mKey.C_Str(), "$mat.twosided") == 0) {
                bool twoSide = false;
                material->Get(AI_MATKEY_TWOSIDED, twoSide);
                std::cerr << twoSide << " ";
            }
        } else {
            std::cerr << "unknown type";
        }
        std::cerr << "\n";
    }
}

} // namespace CS
