#include "CameraManipulator.h"
#include "AxisAlignedBoundingBox.h"
#include "scene/Camera.h"
#include <cassert>

namespace CS
{
CameraManipulator::CameraManipulator(std::shared_ptr<Camera> camera, const Size2u& viewport)
    : m_camera(camera), m_viewport(viewport)
{
    m_aspectRatio = m_viewport.AspectRatioWH();
    UpdateCamera();
}

void CameraManipulator::SetViewport(const Size2u& viewport)
{
    m_viewport = viewport;
    m_aspectRatio = m_viewport.AspectRatioWH();
    UpdateCamera();
}

void CameraManipulator::LookAt(const Vector3f& eye, const Vector3f& center)
{
    m_position = eye;
    m_center = center;
    UpdateCamera();
}

void CameraManipulator::Dolly(float delta, float speed)
{
    auto direction = m_center - m_position;
    auto length = direction.Length();
    // Prevent too fast movement by auto-adjusting speed based on distance to center
    auto dollyAmount = delta * speed * std::max(length * 0.1f, 0.1f);
    auto position = m_position + direction.Normalized() * dollyAmount;

    auto newDirection = m_center - position;
    if (newDirection.Dot(direction) > 0.0f && newDirection.Length() > m_nearPlane) {
        m_position = position;
        m_farPlane = std::max(m_minFarPlane, 2.0f * newDirection.Length());
    }
    UpdateCamera();
}

void CameraManipulator::Zoom(float delta, float speed)
{
    m_fovY -= delta * speed;
    m_fovY = std::clamp(m_fovY, 30.0f, 120.0f);
    UpdateCamera();
}

void CameraManipulator::BeginRotate()
{
    m_bookmark = Bookmark{m_position, m_center, m_up, m_fovY};
}

void CameraManipulator::RotateTrack(const Vector2f& delta, float speed)
{
    if (!m_bookmark.has_value()) {
        return;
    }
    // xyz顺序: pitch (X), yaw (Y), roll (Z)

    auto yaw = speed * (-delta.X() / m_viewport.width);
    auto pitch = speed * (delta.Y() / m_viewport.height);

    const auto& [position, center, up, _] = m_bookmark.value();
    Vector3f refDirection = position - center;

    auto rotationY = Math::Rotation(up.Normalized(), yaw);
    auto rotationX = Math::Rotation(refDirection.Cross(up).Normalized(), pitch);
    auto rotationMat = rotationY * rotationX;

    Vector3f rotatedDir = rotationMat * (m_position - m_center);

    m_position = m_center + rotatedDir;
    // m_up = rotationMat * m_up;

    UpdateCamera();
}

void CameraManipulator::RotatePose(const Vector2f& delta, float speed)
{
    // xyz顺序: pitch (X), yaw (Y), roll (Z)

    auto yaw = speed * (-delta.X() / m_viewport.width);
    auto pitch = speed * (delta.Y() / m_viewport.height);

    Vector3f direction = m_position - m_center;

    auto rotationMat = Math::Rotation(Vector3f{pitch, yaw, 0.0f});

    Vector3f rotatedDir = rotationMat * direction;

    m_center = m_position - rotatedDir;
    m_up = rotationMat * m_up;

    UpdateCamera();
}

void CameraManipulator::EndRotate()
{
    m_bookmark.reset();
}

void CameraManipulator::FlyMove(const Vector2f& delta, float speed)
{
    auto direction =
        speed * (-delta.X() * m_up.Cross(m_position - m_center).Normalized() + delta.Y() * m_up.Normalized());

    m_position += direction;
    m_center += direction;
    UpdateCamera();
}

void CameraManipulator::AlongAxisMove(const Vector2f& delta, float speed)
{
    auto direction = speed * (-delta.X() * Vector3f{1.0f, 0.0f, 0.0f} + delta.Y() * Vector3f{0.0f, 1.0f, 0.0f});

    m_position += direction;
    m_center += direction;
    UpdateCamera();
}

void CameraManipulator::Reset() {}

void CameraManipulator::UpdateCamera()
{
    auto _camera = camera();
    _camera->LookAt(m_position, m_center, m_up);
    _camera->Perspective(Math::AngleToRadian(m_fovY), m_aspectRatio, m_nearPlane, m_farPlane);
}

void CameraManipulator::FitTo(const AABB& box)
{
    m_center = box.GetCenter().Cast<float>();
    auto boxSize = box.GetSize().Cast<float>();
    auto& [min, max] = box;
    m_minFarPlane = static_cast<float>(box.GetDiagonalLength());
    m_position = m_center + Vector3f{0.0f, 0.0f, m_minFarPlane * 1.1f};
    m_farPlane = (m_position - m_center).Length() * 2.0f;

    UpdateCamera();
}

inline std::shared_ptr<Camera> CameraManipulator::camera() const
{
    return m_camera.lock();
}

} // namespace CS
