#pragma once
#include "MetaDefine.h"
#include "IComponent.h"
#include "math/Matrix.h"

namespace CS
{
class CameraSystem;

class [[Meta()]] CameraComponent : public IComponent
{
public:
    friend void Register_CameraComponent();

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
    [[Property(UIName = NearPlane)]]
    float m_nearPlane{0.1f};
    [[Property(UIName = FarPlane)]]
    float m_farPlane{1000.0f};
    [[Property(UIName = Top)]]
    float m_top{100.0f};
    [[Property(UIName = Bottom)]]
    float m_bottom{-100.0f};
    [[Property(UIName = Left)]]
    float m_left{-100.0f};
    [[Property(UIName = Right)]]
    float m_right{100.0f};
    Matrix4f m_projectionMatrix;
    ProjectionType m_projectionType{ProjectionType::Perspective};
    bool m_dirty : 1 {false};
};

} // namespace CS
