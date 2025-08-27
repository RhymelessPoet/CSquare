#include "CameraComponent.h"
#include "base/math/Math.h"

namespace CS
{
CameraComponent::CameraComponent(std::shared_ptr<SceneObject> owner) : IComponent(std::move(owner)) {}

void CameraComponent::OnUpdate()
{
    if (m_dirty) {
        updateProjectionMatrix();
    }
}

void CameraComponent::Ortho(float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
    m_projectionType = ProjectionType::Ortho;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    m_left = left;
    m_right = right;
    m_bottom = bottom;
    m_top = top;
    updateProjectionMatrix();
}

void CameraComponent::Perspective(float fov, float aspectRatio, float nearPlane, float farPlane)
{
    m_projectionType = ProjectionType::Perspective;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    // Assuming fov is in radians, calculate the top and bottom based on aspect ratio
    m_top = nearPlane * std::tan(fov / 2.0f);
    m_bottom = -m_top;
    m_left = -m_top * aspectRatio;
    m_right = m_top * aspectRatio;

    updateProjectionMatrix();
}

const Matrix4f& CameraComponent::GetProjectionMatrix() const
{
    return m_projectionMatrix;
}

const Matrix4f& CameraComponent::GetProjectionMatrix()
{
    if (m_dirty) {
        updateProjectionMatrix();
    }
    return m_projectionMatrix;
}

void CameraComponent::updateProjectionMatrix()
{
    if (m_projectionType == ProjectionType::Ortho) {
        updateOrthoMatrix();
    } else if (m_projectionType == ProjectionType::Perspective) {
        updatePerspectiveMatrix();
    }
    m_dirty = false;
}

void CameraComponent::updateOrthoMatrix()
{
    float rl = m_right - m_left;
    float tb = m_top - m_bottom;
    float fn = m_farPlane - m_nearPlane;

    // clang-format off
    auto data = std::array<float, 16>{
        2.0f / rl, 0.0f, 0.0f, -(m_right + m_left) / rl,
        0.0f, 2.0f / tb, 0.0f, -(m_top + m_bottom) / tb,
        0.0f, 0.0f, -2.0f / fn, -(m_farPlane + m_nearPlane) / fn,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    // clang-format on
    m_projectionMatrix = Matrix4f(data);
}

void CameraComponent::updatePerspectiveMatrix()
{
    float f = m_nearPlane / m_top;
    float aspectRatio = (m_right - m_left) / (m_top - m_bottom);

    // clang-format off
    auto data = std::array<float, 16>{
        f / aspectRatio, 0.0f, (m_left + m_right) / (m_left - m_right), 0.0f,
        0.0f, f, (m_bottom + m_top) / (m_bottom - m_top), 0.0f,
        0.0f, 0.0f, (m_farPlane + m_nearPlane) / (m_nearPlane - m_farPlane), (2.0f * m_farPlane * m_nearPlane) / (m_nearPlane - m_farPlane),
        0.0f, 0.0f, -1.0, 0.0f
    };
    // clang-format on
    m_projectionMatrix = Matrix4f(data);
}

} // namespace CS
