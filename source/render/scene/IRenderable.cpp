#include "IRenderable.h"
#include "IRenderSystem.h"
#include "SceneObject.h"
#include "Transform.h"

namespace CS
{
const Transform& IRenderable::transform() const
{
    return GetComponent<Transform>(owner());
}

Transform& IRenderable::transform()
{
    // Delegate to the const overload and cast away const to avoid duplicating
    // component lookup logic while preserving const-correctness.
    // return const_cast<Transform&>(static_cast<const IRenderable*>(this)->transform());
    auto so = owner();
    return GetComponent<Transform>(so);
}

} // namespace CS
