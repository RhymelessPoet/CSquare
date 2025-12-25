#pragma once
#include "base/math/Matrix.h"
#include <memory>

namespace CS
{
class Scene;
class SceneObject;
class CameraComponent;
class Transform;

class Camera : public std::enable_shared_from_this<Camera>
{
public:
    Camera(std::shared_ptr<Scene> scene);
    ~Camera() = default;

    void LookAt(const Vector3f& eye, const Vector3f& center, const Vector3f& up);

    void Ortho(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    void Perspective(float fov, float aspectRatio, float nearPlane, float farPlane);

    Matrix4f GetViewMatrix() const;
    const Matrix4f& GetProjectionMatrix() const;

    const Vector3f& GetPosition() const;

private:
    inline CameraComponent& cameraComponent();
    inline Transform& transform();
    inline const CameraComponent& cameraComponent() const;
    inline const Transform& transform() const;

private:
    std::shared_ptr<Scene> m_scene;
    std::shared_ptr<SceneObject> m_sceneObject;
};

} // namespace CS
