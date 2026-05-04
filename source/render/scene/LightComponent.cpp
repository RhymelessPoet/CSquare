#include "LightComponent.h"
#include "CameraComponent.h"
#include "SceneObject.h"
#include "SceneObjectEvents.h"
#include "SystemContext.h"
#include "Transform.h"
#include "core/event/IEventDispatcher.h"

namespace CS
{
LightComponent::LightComponent(std::shared_ptr<SceneObject> owner) : IComponent(std::move(owner)) {}

void LightComponent::SetLightType(ELightType type)
{
    m_lightType = type;
}

void LightComponent::SetColor(const Vector3f& color)
{
    m_color = color;
}

const Vector3f& LightComponent::GetColor() const
{
    return m_color;
}

void LightComponent::SetIntensity(float intensity)
{
    m_intensity = intensity;
}

float LightComponent::GetIntensity() const
{
    return m_intensity;
}

void LightComponent::OnUpdate(SystemContext& context)
{
    auto eventDispatcher = context.GetEventDispatcher();
    auto stateNew = EComponentState::Make<"New">();
    if (isOn(stateNew)) {
        eventDispatcher->PushEvent<NewLight>(this);
        setState(stateNew, false);
    }
    update();
}

void LightComponent::update()
{
    auto camera = owner()->GetComponent<CameraComponent>();
}

} // namespace CS
