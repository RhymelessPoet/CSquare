#pragma once
#include "base/Size.h"
#include "base/math/Math.h"
#include <memory>

namespace CS
{

class Camera;

class CameraManipulator
{
public:
    CameraManipulator(std::shared_ptr<Camera> camera, const Size2u& viewport);

    void SetViewport(const Size2u& viewport);

    void LookAt(const Vector3f& eye, const Vector3f& center);

    void Dolly(float delta, float speed = 0.05f);

    void Zoom(float delta, float speed = 0.01f);

    void RotateTrack(const Vector2f& delta, float speed = 2.0f);

    void RotatePose(const Vector2f& delta, float speed = 1.0f);

    void FlyMove(const Vector2f& delta, float speed = 1.0f);

    void AlongAxisMove(const Vector2f& delta, float speed = 1.0f);

    void Reset();

    void UpdateCamera();

protected:
    inline std::shared_ptr<Camera> camera() const;

private:
    std::weak_ptr<Camera> m_camera;
    Size2u m_viewport;
    Vector3f m_center;
    Vector3f m_position;
    Vector3f m_up{0.0f, 1.0f, 0.0f};
    float m_fovY{45.0f};
    float m_aspectRatio{1.0f};
    float m_nearPlane{0.1f};
    float m_farPlane{100.0f};
};

} // namespace CS
