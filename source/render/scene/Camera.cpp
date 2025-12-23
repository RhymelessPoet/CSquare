#include "Camera.h"
#include "CameraComponent.h"
#include "CameraSystem.h"
#include "Scene.h"
#include "SceneObject.h"
#include "Transform.h"
#include "TransformSystem.h"
#include "base/math/Math.h"
#include <cassert>

namespace CS
{
Camera::Camera(std::shared_ptr<Scene> scene) : m_scene(std::move(scene))
{
    m_sceneObject = m_scene->CreateSceneObject();
    m_sceneObject->SetName("Camera");
    m_scene->GetSystem<CameraSystem>().CreateComponent<CameraComponent>(m_sceneObject);
    m_scene->GetSystem<TransformSystem>().CreateComponent<Transform>(m_sceneObject);
}

void Camera::LookAt(const Vector3f& eye, const Vector3f& center, const Vector3f& up)
{
    auto& _transform = transform();
    _transform.SetPosition(eye);
    auto zAxis = (eye - center).Normalized();
    Vector3f xAxis = up.Cross(zAxis).Normalized();
    Vector3f yAxis = zAxis.Cross(xAxis);
    _transform.SetRotation(xAxis, yAxis, zAxis);
}

void Camera::Ortho(float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
    cameraComponent().Ortho(left, right, bottom, top, nearPlane, farPlane);
}

void Camera::Perspective(float fov, float aspectRatio, float nearPlane, float farPlane)
{
    cameraComponent().Perspective(fov, aspectRatio, nearPlane, farPlane);
}

Matrix4f Camera::GetViewMatrix() const
{
    return transform().GetWorldMatrix().Inversed();
}

const Matrix4f& Camera::GetProjectionMatrix() const
{
    return cameraComponent().GetProjectionMatrix();
}

inline CameraComponent& Camera::cameraComponent()
{
    return GetComponent<CameraComponent>(m_sceneObject);
}

inline Transform& Camera::transform()
{
    return GetComponent<Transform>(m_sceneObject);
}

inline const CameraComponent& Camera::cameraComponent() const
{
    return GetComponent<CameraComponent>(m_sceneObject);
}

inline const Transform& Camera::transform() const
{
    return GetComponent<Transform>(m_sceneObject);
}

} // namespace CS
