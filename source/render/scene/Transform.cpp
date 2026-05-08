#include "Transform.h"
#include "SceneObject.h"
#include "base/math/Math.h"

namespace CS
{
Transform::Transform(std::shared_ptr<SceneObject> owner) : IComponent(std::move(owner))
{
    m_dirty = false;
}

Transform::Transform(std::shared_ptr<SceneObject> owner, const Vector3f& position) : Transform(std::move(owner))
{
    m_position = position;
}

void Transform::OnUpdate(SystemContext& context)
{
    update();
}

bool Transform::IsFresh() const
{
    return m_fresh;
}

void Transform::SetPosition(const Vector3f& position)
{
    m_position = position;
    m_dirty = true;
}

const Vector3f& Transform::GetPosition() const
{
    return m_position;
}

void Transform::SetRotation(const Vector3f& rotation)
{
    m_rotation = rotation;
    m_dirty = true;
}

const Vector3f& Transform::GetRotation() const
{
    return m_rotation;
}

Matrix4f Transform::GetRotationMatrix() const
{
    return Math::RotationToMatrix4(m_rotation);
}

void Transform::SetScale(const Vector3f& scale)
{
    m_scale = scale;
    m_dirty = true;
}

const Vector3f& Transform::GetScale() const
{
    return m_scale;
}

Matrix4f CS::Transform::GetLocalModelMatrix() const
{
    return Math::Translation(m_position) * Math::RotationToMatrix4(m_rotation) * Math::Scaling(m_scale);
}

const Matrix4f& Transform::GetWorldMatrix() const
{
    return const_cast<Transform*>(this)->GetWorldMatrix();
}

const Matrix4f& Transform::GetWorldMatrix()
{
    if (m_dirty) {
        update();
    }
    return m_worldMatrix;
}

OBB Transform::Trans(const AABB& box)
{
    if (!box.IsValid()) {
        return OBB::Invalid();
    }
    const auto& [min, max] = box;
    auto aabbCenter = box.GetCenter();
    auto aabbExtents = box.GetSize();

    const Matrix4f& worldMat = GetWorldMatrix();

    auto obbCenter = Math::Transform(worldMat, aabbCenter.Cast<float>());

    std::array<Vector3d, 3> obbAxes = Math::EulerAnglesToAxes<double, float>(GetRotation());

    auto obbExtents = aabbExtents * m_scale.Cast<double>();

    return OBB(obbCenter.Cast<double>(), obbAxes[0], obbAxes[1], obbAxes[2],
               Size3d{obbExtents.X(), obbExtents.Y(), obbExtents.Z()});
}

void Transform::update()
{
    Transform* parentTransform = nullptr;
    auto parentSO = owner()->GetParent();
    bool parentTransformIsFresh = false;
    if (parentSO != nullptr) {
        parentTransform = parentSO->GetComponent<Transform>();
        parentTransformIsFresh = parentTransform != nullptr && parentTransform->IsFresh();
    }

    if (m_dirty || parentTransformIsFresh) {
        m_worldMatrix = GetLocalModelMatrix();

        if (parentTransform != nullptr) {
            m_worldMatrix = parentTransform->GetWorldMatrix() * m_worldMatrix;
        }
        m_dirty = false;
        m_fresh = true;
    }
}

void Transform::notifyChildrenDirty()
{
    for (const auto& child : owner()->GetChildren()) {
        if (auto childTransform = child->GetComponent<Transform>()) {
            childTransform->m_dirty = true;
            childTransform->notifyChildrenDirty();
        }
    }
}

} // namespace CS
