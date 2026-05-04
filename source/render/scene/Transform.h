#pragma once
#include "IComponent.h"
#include "base/math/Matrix.h"
#include "utils/AxisAlignedBoundingBox.h"
#include "utils/OrientedBoundingBox.h"

namespace CS
{

class TransformSystem;

class Transform : public IComponent
{
public:
    using SystemType = TransformSystem;
    Transform(std::shared_ptr<SceneObject> owner);
    Transform(std::shared_ptr<SceneObject> owner, const Vector3f& position);
    ~Transform() = default;

    virtual void OnUpdate(SystemContext& context) override;

    bool IsFresh() const;

    void SetPosition(const Vector3f& position);
    const Vector3f& GetPosition() const;

    void SetRotation(const Vector3f& rotation);
    void SetRotation(const Vector3f& xAxis, const Vector3f& yAxis, const Vector3f& zAxis);

    const Vector3f GetRotation() const;
    const Matrix4f& GetRotationMatrix() const { return m_rotation; }

    void SetScale(const Vector3f& scale);
    const Vector3f& GetScale() const;

    Matrix4f GetLocalModelMatrix() const;
    const Matrix4f& GetWorldMatrix() const;
    const Matrix4f& GetWorldMatrix();

    OBB Trans(const AABB& box);

private:
    void update();
    void notifyChildrenDirty();

private:
    Vector3f m_position{0.0f, 0.0f, 0.0f};
    Matrix4f m_rotation;
    Vector3f m_scale{1.0f, 1.0f, 1.0f};
    Matrix4f m_worldMatrix;
    bool m_dirty : 1;
    bool m_fresh : 1;
};

} // namespace CS
