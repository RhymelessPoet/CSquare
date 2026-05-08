#include "AssimpLoader.h"
#include "asset/AssetNode.h"
#include "asset/AssetScene.h"
#include "asset/BuiltInMaterials.h"
#include "asset/Image.h"
#include "assimp/GltfMaterial.h"
#include "assimp/version.h"
#include "base/Logger.h"
#include "base/math/Math.h"
#include "geometry/Mesh.h"
#include "materials/ImageTexture.h"
#include "materials/Material.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>

using namespace std::literals;

namespace
{
// clang-format off
constexpr std::array<std::pair<aiTextureType, std::string_view>, 7> PBRTextureTypes = {{
    {aiTextureType_BASE_COLOR, "base_color_map"sv},
    {aiTextureType_METALNESS, "metallic_map"sv},
    {aiTextureType_DIFFUSE_ROUGHNESS, "roughness_map"sv},
    // {aiTextureType_AMBIENT_OCCLUSION, "ambient_occlusion_map"sv},
    {aiTextureType_EMISSION_COLOR, "emission_color_map"sv},
    {aiTextureType_SPECULAR, "specular_color_map"sv},
    {aiTextureType_SHININESS, "glossiness_map"sv},
    {aiTextureType_NORMALS, "normal_map"sv}
}};
// clang-format on

std::string_view GetPBRTextureControlName(aiTextureType type)
{
    // clang-format off
    constexpr std::array<std::pair<aiTextureType, std::string_view>, 7> PBRTextureControls = {{
        {aiTextureType_BASE_COLOR, "use_base_color_map"sv},
        {aiTextureType_METALNESS, "use_metallic_map"sv},
        {aiTextureType_DIFFUSE_ROUGHNESS, "use_roughness_map"sv},
        // {aiTextureType_AMBIENT_OCCLUSION, "use_ambient_occlusion_map"sv},
        {aiTextureType_EMISSION_COLOR, "use_emission_color_map"sv},
        {aiTextureType_SPECULAR, "use_specular_map"sv},
        {aiTextureType_SHININESS, "use_glossiness_map"sv},
        {aiTextureType_NORMALS, "use_normal_map"sv}
    }};
    // clang-format on

    for (const auto& [texType, name] : PBRTextureControls) {
        if (texType == type) {
            return name;
        }
    }

    CS::LogError(::CS::BuiltInChannels::Asset(), CS::Fmt("Unknown aiTextureType: {}", static_cast<int>(type)));

    return ""sv;
}

CS::AddressMode GetAddressMode(aiTextureMapMode mode)
{
    static std::array<CS::AddressMode, 4u> AddressModes = {CS::AddressMode::Repeat, CS::AddressMode::ClampToEdge,
                                                           CS::AddressMode::MirroredRepeat, CS::AddressMode::Decal};
    if (mode < 4) {
        return AddressModes[mode];
    }
    CS::LogError(::CS::BuiltInChannels::Asset(), CS::Fmt("Unknown aiTextureMapMode: {}", static_cast<int>(mode)));
    return CS::AddressMode::Repeat;
}

template <typename T>
static T GetProperty(const aiMaterial* material, const char* key, unsigned int type, unsigned int idx)
{
    T value;
    auto ret = material->Get(key, type, idx, value);
    if (ret != aiReturn_SUCCESS) {
        CS::LogError(::CS::BuiltInChannels::Asset(), CS::Fmt("aiMaterial::Get failed for key '{}'", key));
    }
    return value;
}

} // namespace

namespace CS
{

AssimpLoader::AssimpLoader() {}

std::shared_ptr<AssetScene> AssimpLoader::Load(const Path& path)
{
    auto t0 = std::chrono::steady_clock::now();

    CS::LogInfo(::CS::BuiltInChannels::Asset(),
                CS::Fmt("Assimp version: {}.{}.{}", aiGetVersionMajor(), aiGetVersionMinor(), aiGetVersionRevision()));
    Assimp::Importer importer;

    auto postprocessFlags = aiProcess_Triangulate | aiProcess_SortByPType;

    auto ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
    bool isGltf = (ext == ".gltf" || ext == ".glb");

    if (!isGltf) {
        postprocessFlags |= aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace;
    }

    const aiScene* aiscene = importer.ReadFile(path.string(), postprocessFlags);

    if (aiscene == nullptr) {
        CS::LogError(::CS::BuiltInChannels::Asset(), CS::Fmt("Assimp error: {}", importer.GetErrorString()));
        return nullptr;
    }

    if (aiscene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        CS::LogError(::CS::BuiltInChannels::Asset(), CS::Fmt("Assimp incomplete scene: {}", importer.GetErrorString()));
    }

    auto tRead = std::chrono::steady_clock::now();
    CS::LogPerf(::CS::BuiltInChannels::Asset(), CS::Fmt("Assimp::ReadFile completed in {:.2f}ms",
                                                        std::chrono::duration<double, std::milli>(tRead - t0).count()));

    std::vector<std::shared_ptr<Image>> textures;
    std::vector<std::shared_ptr<MaterialInstance>> materials;
    std::vector<std::shared_ptr<Mesh>> meshs;
    std::vector<uint32_t> meshMaterialIndices;

    auto name = FileSystem::is_directory(path) ? path.filename() : path.parent_path().filename();
    auto scene = std::make_shared<AssetScene>(name.string(), path);

    bool noError = true;

    {
        auto t1 = std::chrono::steady_clock::now();
        noError = noError && parseTextures(aiscene, scene);
        CS::LogPerf(::CS::BuiltInChannels::Asset(),
                    CS::Fmt("parseTextures completed in {:.2f}ms",
                            std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t1).count()));
    }
    {
        auto t1 = std::chrono::steady_clock::now();
        noError = noError && parseMaterials(aiscene, scene);
        CS::LogPerf(::CS::BuiltInChannels::Asset(),
                    CS::Fmt("parseMaterials completed in {:.2f}ms",
                            std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t1).count()));
    }
    {
        auto t1 = std::chrono::steady_clock::now();
        noError = noError && parseMeshs(aiscene, scene);
        CS::LogPerf(::CS::BuiltInChannels::Asset(),
                    CS::Fmt("parseMeshs completed in {:.2f}ms",
                            std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t1).count()));
    }
    {
        auto t1 = std::chrono::steady_clock::now();
        noError = noError && parseNodes(aiscene->mRootNode, scene);
        CS::LogPerf(::CS::BuiltInChannels::Asset(),
                    CS::Fmt("parseNodes completed in {:.2f}ms",
                            std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t1).count()));
    }

    CS::LogPerf(::CS::BuiltInChannels::Asset(),
                CS::Fmt("AssimpLoader::Load total: {:.2f}ms for '{}' ({} meshes, {} nodes)",
                        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count(),
                        name.string(), aiscene->mNumMeshes, scene->GetRoot()->GetChildren().size()));

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
            CS::LogError(::CS::BuiltInChannels::Asset(), CS::Fmt("Null texture at index {}", index));
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
            CS::LogError(::CS::BuiltInChannels::Asset(), CS::Fmt("Null material at index {}", index));
            continue;
        }
        if (CS::Logger::Instance().ShouldLog(CS::LogLevels::Trace())) {
            printMaterialInfo(aimaterial);
        }
        const auto mode = GetProperty<int>(aimaterial, AI_MATKEY_SHADING_MODEL);
        if (mode == aiShadingMode_PBR_BRDF) {
            auto mat = parsePBR(aimaterial, scene);
            scene->AddMaterial(std::move(mat));
        }
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

    size_t estimatedBytes = aimesh->mNumVertices * sizeof(float) *
                            ((aimesh->mVertices ? 3u : 0u) + (aimesh->mNormals ? 3u : 0u) +
                             (aimesh->mTangents ? 3u : 0u) + (aimesh->mBitangents ? 3u : 0u));
    for (uint32_t i = 0u; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++i) {
        if (aimesh->mTextureCoords[i] == nullptr)
            break;
        estimatedBytes += aimesh->mNumVertices * 2 * sizeof(float);
    }
    for (uint32_t i = 0u; i < AI_MAX_NUMBER_OF_COLOR_SETS; ++i) {
        if (aimesh->mColors[i] == nullptr)
            break;
        estimatedBytes += aimesh->mNumVertices * 4 * sizeof(float);
    }

    Buffer vertexBuffer;
    vertexBuffer.Reserve(estimatedBytes);
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
        std::vector<aiVector2D> uvs;
        uvs.reserve(aimesh->mNumVertices);
        for (uint32_t i = 0u; i < aimesh->mNumVertices; ++i) {
            aiVector3D uvw = aimesh->mTextureCoords[texCoordIndex][i];
            uvs.emplace_back(uvw.x, uvw.y);
        }
        vertexBuffer.PushBack(uvs.data(), uvs.size());
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
    indices.reserve(3 * aimesh->mNumFaces);

    for (uint32_t faceIndex = 0u; faceIndex < aimesh->mNumFaces; ++faceIndex) {
        auto& aiface = aimesh->mFaces[faceIndex];
        indices.insert(indices.end(), aiface.mIndices, aiface.mIndices + aiface.mNumIndices);
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

std::shared_ptr<MaterialInstance> AssimpLoader::parsePBR(const aiMaterial* aimaterial,
                                                         const std::shared_ptr<AssetScene>& scene)
{
    auto material = BuiltInMaterials::Instance().GetPBRMaterial();
    auto materialInstance = material->CreateInstance();

    materialInstance->SetName(aimaterial->GetName().C_Str());
    parsePBRPTextures(aimaterial, materialInstance, scene);
    parsePBRProperties(aimaterial, materialInstance);

    return materialInstance;
}

bool AssimpLoader::parsePBRProperties(const aiMaterial* aimaterial, const std::shared_ptr<MaterialInstance>& material)
{
    bool noError{true};

    aiColor4D color;
    if (aimaterial->Get(AI_MATKEY_BASE_COLOR, color) == AI_SUCCESS) {
        noError = noError && material->SetUniformValue("base_color", Vector4f{color.r, color.g, color.b, color.a});
    }

    if (aimaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
        noError = noError && material->SetUniformValue("diffuse_color", Vector4f{color.r, color.g, color.b, color.a});
    }

    if (aimaterial->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
        noError = noError && material->SetUniformValue("specular_color", Vector4f{color.r, color.g, color.b, color.a});
        // noError = noError && material->SetUniformValue("use_spec_gloss", true);
    }

    if (aimaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS) {
        noError = noError && material->SetUniformValue("emission_color", Vector4f{color.r, color.g, color.b, color.a});
    }

    float metallic = 0.0f;
    if (aimaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
        noError = noError && material->SetUniformValue("metallic", metallic);
    }

    float roughness = 1.0f;
    if (aimaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS) {
        noError = noError && material->SetUniformValue("roughness", roughness);
    }
    float glossiness = 0.0f;
    if (aimaterial->Get(AI_MATKEY_GLOSSINESS_FACTOR, glossiness) == AI_SUCCESS) {
        noError = noError && material->SetUniformValue("glossiness", glossiness);
        noError = noError && material->SetUniformValue("use_spec_gloss", true);
    }

    return noError;
}

bool AssimpLoader::parsePBRPTextures(const aiMaterial* aimaterial,
                                     const std::shared_ptr<MaterialInstance>& material,
                                     const std::shared_ptr<AssetScene>& scene)
{
    bool noError{true};

    for (const auto& [type, name] : PBRTextureTypes) {
        aiString texPath;
        aiTextureMapping mapping;
        unsigned int uvIndex = 0;
        float blend = 1.0f;
        aiTextureOp op = aiTextureOp_Add;
        aiTextureMapMode mapMode[2] = {aiTextureMapMode_Wrap, aiTextureMapMode_Wrap};

        if (aiTextureType_NORMAL_CAMERA == type) {
            int a = 0;
        }

        auto controlName = GetPBRTextureControlName(type);
        if (!material->SetUniformValue(controlName, false)) {
            CS::LogError(::CS::BuiltInChannels::Asset(),
                         CS::Fmt("Failed to set texture control uniform '{}'", std::string(controlName)));
        }

        if (AI_SUCCESS != aimaterial->GetTexture(type, 0, &texPath, &mapping, &uvIndex, &blend, &op, mapMode)) {
            continue;
        }
        auto imagePath = scene->GetPath().parent_path().append(texPath.C_Str());
        auto image = scene->GetTexture(imagePath.string());
        if (image == nullptr) {
            image = std::make_shared<Image>(imagePath);
            scene->AddTexture(image);
        }
        auto texture = material->GetInstanceTexture(name);
        if (texture == nullptr) {
            continue;
        }
        auto insTexture = texture->Clone();
        if (auto imageTexture = dynamic_cast<ImageTexture*>(insTexture.get()); imageTexture != nullptr) {
            imageTexture->SetImage(image);
            imageTexture->SetAddressModeUV(GetAddressMode(mapMode[0]), GetAddressMode(mapMode[1]));
            if (!material->SetUniformValue(controlName, true)) {
                noError = false;
            }
            if (!material->SetTexture(name, std::move(insTexture))) {
                noError = false;
            }
            if (name == "base_color_map"sv) {
                // imageTexture->SetSRGB(true);
            }
            if (float normalScale = 1.0f;
                name == "normal_map"sv &&
                aimaterial->Get(AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_NORMALS, 0), normalScale) == AI_SUCCESS)
            {
                noError = noError && material->SetUniformValue("normal_scale", normalScale);
            }
            if (name == "specular_color_map"sv || name == "glossiness_map"sv) {
                noError = noError && material->SetUniformValue("use_spec_gloss", true);
            }
        }
    }

    return noError;
}

void AssimpLoader::printMaterialInfo(const aiMaterial* material)
{
    CS::LogTrace(::CS::BuiltInChannels::Asset(), CS::Fmt("--- Parse Material: {} ---", material->GetName().C_Str()));
    for (uint32_t index = 0u; index < material->mNumProperties; ++index) {
        auto aiproperty = material->mProperties[index];
        std::string value;
        if (aiproperty->mType == aiPTI_Float) {
            auto count = aiproperty->mDataLength / sizeof(float);
            auto data = reinterpret_cast<float*>(aiproperty->mData);
            for (uint32_t i = 0u; i < count; ++i) {
                value += std::format("{} ", data[i]);
            }
        } else if (aiproperty->mType == aiPTI_String) {
            value = reinterpret_cast<char*>(aiproperty->mData);
        } else if (aiproperty->mType == aiPTI_Integer) {
            auto count = aiproperty->mDataLength / sizeof(int);
            auto data = reinterpret_cast<int*>(aiproperty->mData);
            for (uint32_t i = 0u; i < count; ++i) {
                value += std::format("{} ", data[i]);
            }
        } else if (aiproperty->mType == aiPTI_Double) {
            auto count = aiproperty->mDataLength / sizeof(double);
            auto data = reinterpret_cast<double*>(aiproperty->mData);
            for (uint32_t i = 0u; i < count; ++i) {
                value += std::format("{} ", data[i]);
            }
        } else if (aiproperty->mType == aiPTI_Buffer) {
            if (strcmp(aiproperty->mKey.C_Str(), "$mat.twosided") == 0) {
                bool twoSide = false;
                material->Get(AI_MATKEY_TWOSIDED, twoSide);
                value = std::format("{}", twoSide);
            }
        } else {
            value = "unknown type";
        }
        CS::LogTrace(::CS::BuiltInChannels::Asset(), CS::Fmt("  {}: {}", aiproperty->mKey.C_Str(), value));
    }
    for (const auto& [type, name] : PBRTextureTypes) {
        aiString texPath;
        aiTextureMapping mapping;
        unsigned int uvIndex = 0;
        float blend = 1.0f;
        aiTextureOp op = aiTextureOp_Add;
        aiTextureMapMode mapMode[2] = {aiTextureMapMode_Wrap, aiTextureMapMode_Wrap};

        if (AI_SUCCESS == material->GetTexture(type, 0, &texPath, &mapping, &uvIndex, &blend, &op, mapMode)) {
            CS::LogTrace(::CS::BuiltInChannels::Asset(), CS::Fmt("  {} path: {}", name, texPath.C_Str()));
        }
    }
}

} // namespace CS