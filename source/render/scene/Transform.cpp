#include "Transform.h"
#include "SceneObject.h"
#include "base/math/Math.h"

namespace CS
{
namespace
{
// Cached state flag handles to avoid repeated name-lookups in the hot path.
inline EComponentState DirtyState()
{
    static const auto s = EComponentState::Make<"Dirty">();
    return s;
}
inline EComponentState FreshState()
{
    static const auto s = EComponentState::Make<"Fresh">();
    return s;
}
} // namespace

Transform::Transform(std::shared_ptr<SceneObject> owner) : IComponent(std::move(owner))
{
    setState(DirtyState(), true);
    setState(FreshState(), false);
}

Transform::Transform(std::shared_ptr<SceneObject> owner, const Vector3f& position) : Transform(std::move(owner))
{
    m_position = position;
}

void Transform::OnUpdate(SystemContext& context)
{
    setState(FreshState(), false);

    update();
}

bool Transform::IsFresh() const
{
    return IsOn(FreshState());
}

void Transform::SetPosition(const Vector3f& position)
{
    m_position = position;
    setState(DirtyState(), true);
}

const Vector3f& Transform::GetPosition() const
{
    return m_position;
}

void Transform::SetRotation(const Vector3f& rotation)
{
    m_rotation = rotation;
    setState(DirtyState(), true);
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
    setState(DirtyState(), true);
}

const Vector3f& Transform::GetScale() const
{
    return m_scale;
}

Matrix4f CS::Transform::GetLocalModelMatrix() const
{
    // If the cache is clean, return it; otherwise compose on the fly without
    // mutating the cache (that is update()'s job).
    if (!IsOn(DirtyState())) {
        return m_localMatrix;
    }
    Matrix4f out;
    Math::ComposeTRS(out, m_position, m_rotation, m_scale);
    return out;
}

const Matrix4f& Transform::GetWorldMatrix() const
{
    return const_cast<Transform*>(this)->GetWorldMatrix();
}

const Matrix4f& Transform::GetWorldMatrix()
{
    if (IsOn(DirtyState())) {
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

    const bool dirty = IsOn(DirtyState());
    if (dirty || parentTransformIsFresh) {
        // Only rebuild the local matrix when our own TRS changed. If only the
        // parent moved, the cached local matrix is still valid.
        if (dirty) {
            Math::ComposeTRS(m_localMatrix, m_position, m_rotation, m_scale);
        }

        if (parentTransform != nullptr) {
            m_worldMatrix = parentTransform->GetWorldMatrix() * m_localMatrix;
        } else {
            m_worldMatrix = m_localMatrix;
        }
        setState(DirtyState(), false);
        setState(FreshState(), true);
    }
}

void Transform::notifyChildrenDirty()
{
    for (const auto& child : owner()->GetChildren()) {
        if (auto childTransform = child->GetComponent<Transform>()) {
            childTransform->setState(DirtyState(), true);
            childTransform->notifyChildrenDirty();
        }
    }
}

} // namespace CS
