#pragma once
#include "LightType.h"
#include "base/math/Vector.h"
#include <memory>

namespace CS
{
class Scene;
class SceneObject;
class CameraComponent;
class Transform;
class LightComponent;

class Light : public std::enable_shared_from_this<Light>
{
public:
    Light(std::shared_ptr<Scene> scene);
    Light(std::shared_ptr<SceneObject> sceneObject);
    ~Light() = default;

    void SetLightType(ELightType type);
    ELightType GetLightType() const;

    void SetDirection(const Vector3f& direction);
    const Vector3f GetDirection() const;

    void SetColor(const Vector3f& color);
    const Vector3f& GetColor() const;

    void SetIntensity(float intensity);
    float GetIntensity() const;

    void SetPosition(const Vector3f& position);
    const Vector3f& GetPosition() const;

    void SetShadowNormalBias(const Vector2f& bias);
    const Vector2f& GetShadowNormalBias() const;

private:
    inline LightComponent& lightComponent(this Light& self);
    inline const LightComponent& lightComponent(this const Light& self);

    inline Transform& transform(this Light& self);
    inline const Transform& transform(this const Light& self);

private:
    std::shared_ptr<SceneObject> m_sceneObject;
};

} // namespace CS
