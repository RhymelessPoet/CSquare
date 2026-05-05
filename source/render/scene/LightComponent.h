#pragma once
#include "CSMetaDefine.h"
#include "IComponent.h"
#include "LightType.h"
#include "base/math/Vector.h"

namespace CS
{
class LightSystem;
class CameraComponent;
class TransformComponent;
class Light;
class AxisAlignedBoundingBox;

class [[Meta()]] LightComponent : public IComponent
{
public:
    friend void Register_LightComponent();

    using SystemType = LightSystem;

    LightComponent(std::shared_ptr<SceneObject> owner);
    ~LightComponent() = default;

    void SetLightType(ELightType type);
    ELightType GetLightType() const { return m_lightType; }

    void SetColor(const Vector3f& color);
    const Vector3f& GetColor() const;

    void SetIntensity(float intensity);
    float GetIntensity() const;

    void SetShadowNormalBias(const Vector2f& bias) { m_shadowNormalBias = bias; }
    const Vector2f& GetShadowNormalBias() const { return m_shadowNormalBias; }

    void OnUpdate(SystemContext& context) override;

private:
    void update();

private:
    [[Property(Setter = SetColor, Getter = GetColor, UIName = Color)]]
    Vector3f m_color{1.0f, 1.0f, 1.0f};
    Vector2f m_shadowNormalBias{0.001f, 0.001f};
    [[Property(Setter = SetIntensity, Getter = GetIntensity, UIName = Intensity)]]
    float m_intensity{1.0f};
    ELightType m_lightType;
};

} // namespace CS
