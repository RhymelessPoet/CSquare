#include "MeshRenderSystem.h"
#include "Camera.h"
#include "MeshRenderer.h"
#include "geometry/GeometryNode.h"
#include "materials/Material.h"
#include "renderer/MaterialCompiler.h"
#include "renderer/RenderContext.h"

namespace CS
{
void MeshRenderSystem::OnUpdate()
{
    for (auto component : m_components) {
        auto meshRender = dynamic_cast<MeshRenderer*>(component);
        if (meshRender != nullptr) {
            meshRender->OnUpdate();

            auto material = meshRender->GetGeometryNode(0u)->GetMaterial()->GetMaterial();
            m_materials[material->GetID()] = material;
        }
    }
}

void MeshRenderSystem::OnRender(RenderContext& context)
{
    auto camera = context.GetCamera();

    const auto viewMatrix = camera->GetViewMatrix().Transposed();
    const auto projectionMatrix = camera->GetProjectionMatrix().Transposed();
    for (auto& [_, material] : m_materials) {
        auto noError = material->SetUniformValue(std::string_view("projection"), projectionMatrix.ToStdVector());
        noError = noError && material->SetUniformValue(std::string_view("view"), viewMatrix.ToStdVector());

        auto& materialCompiler = context.GetMaterialCompiler();
        material->GetDefaultInstance().Apply(materialCompiler);

        auto shaderBindingSet = materialCompiler.GetShaderBindingSet(material->GetDefaultInstance());
        context.GetCommandBuffer().Bind(shaderBindingSet);
    }

    for (auto& component : m_components) {
        if (auto meshRenderer = dynamic_cast<MeshRenderer*>(component)) {
            meshRenderer->OnRender(context);
        }
    }
}

} // namespace CS
