#include "Camera.h"
#include "CameraComponent.h"
#include "CameraSystem.h"
#include "Scene.h"
#include "SceneObject.h"
#include "Transform.h"
#include "TransformSystem.h"
#include "math/Math.h"
#include "scene/SceneObjectComposer.h"
#include <cassert>

namespace CS
{
Camera::Camera(std::shared_ptr<Scene> scene)
{
    m_sceneObject = scene->CreateSceneObject();
    m_sceneObject->SetName("Camera");

    auto composer = scene->GetComposer();
    composer->AddComponent<CameraComponent>(m_sceneObject);
    composer->AddComponent<Transform>(m_sceneObject);
}

Camera::Camera(std::shared_ptr<SceneObject> sceneObject) : m_sceneObject(std::move(sceneObject))
{
    auto scene = m_sceneObject->GetScene();
    auto composer = scene->GetComposer();

    if (m_sceneObject->GetComponent<CameraComponent>() == nullptr) {
        composer->AddComponent<CameraComponent>(m_sceneObject);
    }
    if (m_sceneObject->GetComponent<Transform>() == nullptr) {
        composer->AddComponent<Transform>(m_sceneObject);
    }
}

void Camera::LookAt(const Vector3f& eye, const Vector3f& center, const Vector3f& up)
{
    auto& _transform = transform();
    _transform.SetPosition(eye);
    auto zAxis = (eye - center).Normalized();
    Vector3f xAxis = up.Cross(zAxis).Normalized();
    Vector3f yAxis = zAxis.Cross(xAxis);
    // Build the rotation matrix from the orthonormal basis and reduce to
    // Euler angles (Transform stores rotation as Vector3f radians).
    Matrix4f m;
    m.SetIdentity();
    m.SetRow(0u, {xAxis[0], xAxis[1], xAxis[2], 0.0f});
    m.SetRow(1u, {yAxis[0], yAxis[1], yAxis[2], 0.0f});
    m.SetRow(2u, {zAxis[0], zAxis[1], zAxis[2], 0.0f});
    _transform.SetRotation(Math::RotationMatrixToAnglesXYZ(m));
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

const Vector3f& Camera::GetPosition() const
{
    return transform().GetPosition();
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
