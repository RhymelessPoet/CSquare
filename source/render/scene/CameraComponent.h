#pragma once
#include "IComponent.h"
#include "base/math/Matrix.h"

namespace CS
{
class CameraSystem;

class CameraComponent : public IComponent
{
public:
    enum class ProjectionType
    {
        Perspective,
        Ortho,
        Max
    };
    using SystemType = CameraSystem;
    explicit CameraComponent(std::shared_ptr<SceneObject> owner);
    ~CameraComponent() = default;

    void OnUpdate(SystemContext& context) override;

    void Ortho(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    void Perspective(float fov, float aspectRatio, float nearPlane, float farPlane);

    const Matrix4f& GetProjectionMatrix() const;
    const Matrix4f& GetProjectionMatrix();

    ProjectionType GetProjectionType() const { return m_projectionType; }

private:
    void updateProjectionMatrix();
    void updateOrthoMatrix();
    void updatePerspectiveMatrix();

private:
    float m_nearPlane{0.1f};
    float m_farPlane{1000.0f};
    float m_top{100.0f};
    float m_bottom{-100.0f};
    float m_left{-100.0f};
    float m_right{100.0f};
    Matrix4f m_projectionMatrix;
    ProjectionType m_projectionType{ProjectionType::Perspective};
    bool m_dirty : 1 {false};
};

} // namespace CS
