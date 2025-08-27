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

void Transform::OnUpdate()
{
    auto parentSO = owner()->GetParent();
    if (parentSO != nullptr && parentSO->GetComponent<Transform>().IsValid()) {
        const auto& parentTransform = dynamic_cast<const Transform&>(parentSO->GetComponent<Transform>());
        if (m_dirty || parentTransform.IsFresh()) {
            m_worldMatrix = parentTransform.GetWorldMatrix() * GetLocalModelMatrix();
            m_dirty = false;
            m_fresh = true;
        }
    } else if (m_dirty) {
        m_worldMatrix = GetLocalModelMatrix();
        m_dirty = false;
        m_fresh = true;
    }
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
    m_rotation = Math::RotationToMatrix4(rotation);
    m_dirty = true;
}

void Transform::SetRotation(const Vector3f& xAxis, const Vector3f& yAxis, const Vector3f& zAxis)
{
    m_rotation.SetRow(0u, {xAxis[0], xAxis[1], xAxis[2], 0.0f});
    m_rotation.SetRow(1u, {yAxis[0], yAxis[1], yAxis[2], 0.0f});
    m_rotation.SetRow(2u, {zAxis[0], zAxis[1], zAxis[2], 0.0f});
}

const Vector3f Transform::GetRotation() const
{
    return Math::RotationMatrixToAnglesXYZ(m_rotation);
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
    return Math::Translation(m_position) * m_rotation * Math::Scaling(m_scale);
}

const Matrix4f& Transform::GetWorldMatrix() const
{
    return m_worldMatrix;
}

const Matrix4f& Transform::GetWorldMatrix()
{
    if (m_dirty) {
        OnUpdate();
    }
    return m_worldMatrix;
}

} // namespace CS
