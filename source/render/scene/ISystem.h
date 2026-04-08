#pragma once
#include "core/event/IEventDispatcher.h"
#include <vector>

namespace CS
{
class IComponent;
class SceneObject;
class InvalidComponent;
class SystemContext;

class ISystem : public IEventDispatcher
{
public:
    friend class IComponent;

    ISystem(/* args */);
    virtual ~ISystem();

    virtual bool IsValid() const { return true; }
    virtual void OnUpdate(SystemContext& context) = 0;

    template <typename T, typename... Args>
    bool CreateComponent(std::shared_ptr<SceneObject> so, Args&&... args)
    {
        static_assert(std::is_base_of_v<IComponent, T>, "T must be derived from IComponent");
        if (dynamic_cast<T::SystemType*>(this) == nullptr) {
            return false; // Ensure the system is compatible with the component type
        }
        auto component = std::make_unique<T>(so, std::forward<Args>(args)...);

        return assembleComponent(std::move(so), std::move(component));
    }

private:
    bool assembleComponent(std::shared_ptr<SceneObject> so, std::unique_ptr<IComponent> component);
    void removeComponent(IComponent* component);

    std::vector<IEventListener*> sift(IEvent* event) const override { return std::vector<IEventListener*>(); }
    std::unique_ptr<IEvent> dispatch(IEventDispatcher* nextDispatcher, std::unique_ptr<IEvent> event) override;

protected:
    std::vector<IComponent*> m_components;
};

class InvalidSystem : public ISystem
{
public:
    using ComponentType = InvalidComponent;

    InvalidSystem() = default;
    InvalidSystem(const InvalidSystem&) = delete;
    InvalidSystem& operator=(const InvalidSystem&) = delete;
    InvalidSystem(InvalidSystem&&) = delete;
    InvalidSystem& operator=(InvalidSystem&&) = delete;

    virtual bool IsValid() const override { return false; }

    virtual void OnUpdate(SystemContext& context) override {};

    static InvalidSystem& Instance();
};

} // namespace CS
