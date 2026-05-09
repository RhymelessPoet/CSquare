#include "Light.h"
#include "CameraComponent.h"
#include "CameraSystem.h"
#include "LightComponent.h"
#include "LightSystem.h"
#include "Scene.h"
#include "SceneObject.h"
#include "SceneObjectComposer.h"
#include "Transform.h"
#include "TransformSystem.h"
#include "base/math/Math.h"

namespace CS
{

struct Rotation
{
    Vector3f xAxis, yAxis, zAxis;

    static Rotation FromZAxis(const Vector3f& zAxis)
    {
        Vector3f normalizedZ = zAxis.Normalized();
        Vector3f up{0.0f, 1.0f, 0.0f};
        if (std::abs(normalizedZ.Dot(up)) > 0.999f) {
            up = {1.0f, 0.0f, 0.0f};
        }
        Vector3f xAxis = up.Cross(normalizedZ).Normalized();
        Vector3f yAxis = normalizedZ.Cross(xAxis).Normalized();
        return {xAxis, yAxis, normalizedZ};
    }
};

Light::Light(std::shared_ptr<Scene> scene)
{
    m_sceneObject = scene->CreateSceneObject();
    m_sceneObject->SetName("Light");

    auto composer = scene->GetComposer();
    composer->AddComponent<LightComponent>(m_sceneObject);
    composer->AddComponent<CameraComponent>(m_sceneObject);
    composer->AddComponent<Transform>(m_sceneObject);

    SetLightType(ELightType::Make<"Directional">());
}

Light::Light(std::shared_ptr<SceneObject> sceneObject) : m_sceneObject(std::move(sceneObject))
{
    auto composer = m_sceneObject->GetScene()->GetComposer();

    if (m_sceneObject->GetComponent<LightComponent>() == nullptr) {
        composer->AddComponent<LightComponent>(m_sceneObject);
    }
    if (m_sceneObject->GetComponent<Transform>() == nullptr) {
        composer->AddComponent<Transform>(m_sceneObject);
    }
}

void Light::SetLightType(ELightType type)
{
    lightComponent().SetLightType(type);
}

ELightType Light::GetLightType() const
{
    return lightComponent().GetLightType();
}

void Light::SetDirection(const Vector3f& direction)
{
    auto [xAxis, yAxis, zAxis] = Rotation::FromZAxis(-direction);
    // Compose an XYZ rotation matrix from the three orthonormal axes, then
    // reduce to Euler angles for the Transform's Vector3f storage.
    Matrix4f m;
    m.SetIdentity();
    m.SetRow(0u, {xAxis[0], xAxis[1], xAxis[2], 0.0f});
    m.SetRow(1u, {yAxis[0], yAxis[1], yAxis[2], 0.0f});
    m.SetRow(2u, {zAxis[0], zAxis[1], zAxis[2], 0.0f});
    transform().SetRotation(Math::RotationMatrixToAnglesXYZ(m));
}

const Vector3f Light::GetDirection() const
{
    auto mat = Math::Extract<3u>(transform().GetRotationMatrix());
    return -mat.GetRow(2u);
}

void Light::SetColor(const Vector3f& color)
{
    lightComponent().SetColor(color);
}

const Vector3f& Light::GetColor() const
{
    return lightComponent().GetColor();
}

void Light::SetIntensity(float intensity)
{
    lightComponent().SetIntensity(intensity);
}

float Light::GetIntensity() const
{
    return lightComponent().GetIntensity();
}

void Light::SetPosition(const Vector3f& position)
{
    transform().SetPosition(position);
}

const Vector3f& Light::GetPosition() const
{
    return transform().GetPosition();
}

void Light::SetShadowNormalBias(const Vector2f& bias)
{
    lightComponent().SetShadowNormalBias(bias);
}

const Vector2f& Light::GetShadowNormalBias() const
{
    return lightComponent().GetShadowNormalBias();
}

bool Light::IsDirty() const
{
    return lightComponent().IsOn(EComponentState::Make<"Dirty">());
}

LightComponent& Light::lightComponent(this Light& self)
{
    return GetComponent<LightComponent>(self.m_sceneObject);
}

const LightComponent& Light::lightComponent(this const Light& self)
{
    return const_cast<Light&>(self).lightComponent();
}

Transform& Light::transform(this Light& self)
{
    return GetComponent<Transform>(self.m_sceneObject);
}

const Transform& Light::transform(this const Light& self)
{
    return const_cast<Light&>(self).transform();
}

} // namespace CS
