#pragma once
#include "base/Macros.h"
#include "base/math/Vector.h"
#include <memory>
#include <vector>

namespace CS
{

class View;
class SceneObject;
class SceneObjectComposer;

class IRenderSample
{
public:
    static Vector3f RandomPosition(const Vector3f& minRange, const Vector3f& maxRange);
    static std::vector<Vector3f> RandomPositions(uint32_t count, const Vector3f& minRange, const Vector3f& maxRange);

    IRenderSample(std::shared_ptr<SceneObjectComposer> composer);

    virtual void Initialize(std::shared_ptr<View> view) = 0;

    virtual void OnUpdate() = 0;

protected:
    bool createMeshRenderer(std::shared_ptr<SceneObject> object);
    bool createTransform(std::shared_ptr<SceneObject> object);

protected:
    std::shared_ptr<SceneObjectComposer> m_composer;
};

} // namespace CS
