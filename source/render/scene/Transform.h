#pragma once
#include "CSMetaDefine.h"
#include "IComponent.h"
#include "base/math/Matrix.h"
#include "utils/AxisAlignedBoundingBox.h"
#include "utils/OrientedBoundingBox.h"

namespace CS
{

class TransformSystem;

class [[Meta()]] Transform : public IComponent
{
public:
    friend void Register_Transform();

    using SystemType = TransformSystem;
    Transform(std::shared_ptr<SceneObject> owner);
    Transform(std::shared_ptr<SceneObject> owner, const Vector3f& position);
    ~Transform() = default;

    virtual void OnUpdate(SystemContext& context) override;

    [[Method()]]
    bool IsFresh() const;

    [[Method()]]
    void SetPosition(const Vector3f& position);
    [[Method()]]
    const Vector3f& GetPosition() const;

    [[Method()]]
    void SetRotation(const Vector3f& rotation);
    [[Method()]]
    void SetRotation(const Vector3f& xAxis, const Vector3f& yAxis, const Vector3f& zAxis);

    [[Method()]]
    const Vector3f GetRotation() const;
    [[Method()]]
    const Matrix4f& GetRotationMatrix() const
    {
        return m_rotation;
    }

    [[Method()]]
    void SetScale(const Vector3f& scale);
    [[Method()]]
    const Vector3f& GetScale() const;

    [[Method()]]
    Matrix4f GetLocalModelMatrix() const;
    [[Method()]]
    const Matrix4f& GetWorldMatrix() const;
    [[Method()]]
    const Matrix4f& GetWorldMatrix();

    [[Method()]]
    OBB Trans(const AABB& box);

private:
    void update();
    void notifyChildrenDirty();

private:
    [[Property(Setter = SetPosition, Getter = GetPosition, UIName = Position)]]
    Vector3f m_position{0.0f, 0.0f, 0.0f};
    [[Property(Getter = GetRotationMatrix, UIName = Rotation)]]
    Matrix4f m_rotation;
    [[Property(Setter = SetScale, Getter = GetScale, UIName = Scale)]]
    Vector3f m_scale{1.0f, 1.0f, 1.0f};
    Matrix4f m_worldMatrix;
    bool m_dirty : 1;
    bool m_fresh : 1;
};

} // namespace CS
