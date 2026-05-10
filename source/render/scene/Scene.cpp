#include "Scene.h"
#include "Camera.h"
#include "IRenderSystem.h"
#include "Light.h"
#include "LightComponent.h"
#include "MeshRenderer.h"
#include "SceneEvents.h"
#include "SceneObject.h"
#include "SceneObjectEvents.h"
#include "base/math/Math.h"
#include "geometry/GeometryNode.h"
#include "materials/Material.h"
#include "renderer/MaterialCompiler.h"
#include "renderer/RenderContext.h"

namespace CS
{

static Vector2f CalculateShadowNormalBias(const Matrix4f& vp, const Vector2f& texelSize)
{
    auto J = Math::Jacobian(vp.Inversed(), {0.0f, 0.0f, 0.0f});

    Vector2f bias = texelSize * Vector2f{J.GetRow(0).Length(), J.GetRow(1).Length()} * 2.0f;

    return bias;
}

Scene::Scene(std::shared_ptr<SceneObjectComposer> composer, std::string_view name)
    : m_composer(std::move(composer)), m_name(name)
{
    m_root = std::make_shared<SceneObject>("Root");
}

Scene::~Scene() {}

void Scene::OnRender(RenderContext& context)
{
    collectRenderables(m_root);

    auto camera = context.GetCamera();

    Matrix4f lightVPMatrix;
    Vector3f lightDirection{1.0f, 1.0f, 1.0f};
    Vector3f lightColor{1.0f, 1.0f, 1.0f};
    float lightIntensity{1.0f};

    Vector2f normalBias;

    lightVPMatrix.SetIdentity();

    if (!m_lights.empty()) {
        auto light = Light(m_lights[0].lock());
        auto lightCamera = Camera(m_lights[0].lock());
        if (context.GetTargetViewType() == EViewType::Make<"Shadow_Map">()) {
            updateShadowCamera(*camera, light);
        }

        auto lightVPMatrix_ = lightCamera.GetProjectionMatrix() * lightCamera.GetViewMatrix();
        lightVPMatrix = lightVPMatrix_.Transposed();
        lightDirection = light.GetDirection();
        lightColor = light.GetColor();
        lightIntensity = light.GetIntensity();

        const auto& [width, height] = context.GetViewport().GetSize();
        normalBias = CalculateShadowNormalBias(lightVPMatrix_, {1.0f / width, 1.0f / height}) * 0.5f;
    }

    const auto& cameraPos = camera->GetPosition();
    const auto viewMatrix = camera->GetViewMatrix().Transposed();
    const auto projectionMatrix = camera->GetProjectionMatrix().Transposed();

    for (auto& [_, material] : m_materials) {
        if (material->GetConfiguration().GetViewType() != context.GetTargetViewType()) {
            continue;
        }

        auto noError = material->SetUniformValue("projection", projectionMatrix.ToStdVector());
        noError = noError && material->SetUniformValue("view", viewMatrix.ToStdVector());
        noError = noError && material->SetUniformValue("light_vp_matrix", lightVPMatrix.ToStdVector());
        noError = noError && material->SetUniformValue("camera_position", cameraPos);
        noError = noError && material->SetUniformValue("light_direction", lightDirection.Normalized());
        noError = noError && material->SetUniformValue("light_color", lightColor);
        noError = noError && material->SetUniformValue("shadow_normal_bias", normalBias);
        noError = noError && material->SetUniformValue("light_intensity", lightIntensity);

        auto& materialCompiler = context.GetMaterialCompiler();

        auto shaderBindingSet = materialCompiler.GetShaderBindingSet(material->GetDefaultInstance());

        material->GetDefaultInstance().Apply(materialCompiler);
        context.GetCommandBuffer().Bind(shaderBindingSet);
    }

    for (const auto renderable : m_renderables) {
        renderable->OnRender(context);
    }
    m_renderables.clear();
}

std::shared_ptr<Camera> Scene::CreateCamera()
{
    return std::make_shared<Camera>(shared_from_this());
}

std::shared_ptr<SceneObject> Scene::CreateSceneObject(std::shared_ptr<SceneObject> parent)
{
    auto sceneObject = std::make_shared<SceneObject>();
    sceneObject->SetScene(shared_from_this());
    if (parent != nullptr) {
        sceneObject->SetParent(parent);
    } else {
        sceneObject->SetParent(m_root);
    }
    return sceneObject;
}

const AABB& Scene::GetAABB(bool reCompute)
{
    if (reCompute) {
        traverseWith(m_root, [this](std::shared_ptr<SceneObject> object) {
            auto meshRenderer = object->GetComponent<MeshRenderer>();
            if (meshRenderer == nullptr) {
                return;
            }
            auto obb = meshRenderer->GetWorldBoundingBox();
            if (obb.IsValid()) {
                m_box.Include(obb);
            }
        });
    }
    return std::as_const(*this).GetAABB();
}

std::vector<std::shared_ptr<SceneObject>> Scene::GetLights(ELightType type) const
{
    std::vector<std::shared_ptr<SceneObject>> lights;
    for (const auto& weakLight : m_lights) {
        if (auto light = weakLight.lock()) {
            if (auto lightComponent = light->GetComponent<LightComponent>()) {
                if (lightComponent->GetLightType() == type) {
                    lights.push_back(light);
                }
            }
        }
    }
    return lights;
}

std::unique_ptr<IEvent> Scene::OnEvent(std::unique_ptr<IEvent> event)
{
    if (auto event_ = dynamic_cast<NewGeometryNode*>(event.get()); event_ != nullptr) {
        return onEvent(event_);
    }
    if (auto event_ = dynamic_cast<NewLight*>(event.get()); event_ != nullptr) {
        return onEvent(event_);
    }
    return event;
}

void Scene::traverseWith(std::shared_ptr<SceneObject> object,
                         const std::function<void(std::shared_ptr<SceneObject>)>& func)
{
    func(object);
    for (const auto& child : object->GetChildren()) {
        if (!child->IsActive()) {
            continue;
        }
        traverseWith(child, func);
    }
}

void Scene::collectRenderables(std::shared_ptr<SceneObject> object)
{
    if (auto meshRenderer = object->GetComponent<MeshRenderer>()) {
        m_renderables.push_back(meshRenderer);
    }

    for (const auto& child : object->GetChildren()) {
        if (!child->IsActive()) {
            continue;
        }
        collectRenderables(child);
    }
}

std::unique_ptr<IEvent> Scene::onEvent(NewGeometryNode* event)
{
    auto insertMaterial = [=](this auto&& self, const std::shared_ptr<Material>& material) -> bool {
        auto result = m_materials.emplace(material->GetID(), material);

        for (const auto& mat : material->GetRequisiteMaterials()) {
            self(mat);
        }
        return result.second;
    };
    auto materialInstance = event->GetGeometryNode()->GetMaterial();
    if (insertMaterial(materialInstance->GetMaterial())) {
        return std::make_unique<NewMaterialInScene>(shared_from_this(), materialInstance->GetMaterial());
    }
    return std::unique_ptr<IEvent>();
}

std::unique_ptr<IEvent> Scene::onEvent(NewLight* event)
{
    if (auto lightComponent = event->GetComponent()) {
        auto light = lightComponent->GetSO();
        m_lights.push_back(light);
        return std::make_unique<NewLightInScene>(shared_from_this(), light);
    }
    return std::unique_ptr<IEvent>();
}

void Scene::updateShadowCamera(Camera& camera, const Light& light)
{
    auto direction = light.GetDirection().Normalized();
    auto center = m_box.GetCenter().Cast<float>();
    auto distance = static_cast<float>(m_box.GetDiagonalLength());
    auto eye = -distance * direction + center;

    camera.LookAt(eye, center, {0.0f, 1.0f, 0.0f});

    auto viewMatrix = camera.GetViewMatrix();
    AABB viewBox;
    for (const auto& point : GetAABB(true).GetCorners()) {
        auto point3f = point.Cast<float>();
        auto pointInView = viewMatrix * Vector4f(point3f, 1.0f);
        viewBox.Include(pointInView.Slice<0, 3>().Cast<double>());
    }

    const auto& [minX, minY, minZ] = viewBox.GetMin();
    const auto& [maxX, maxY, maxZ] = viewBox.GetMax();

    camera.Ortho(minX, maxX, minY, maxY, -maxZ, -minZ);
}

} // namespace CS
