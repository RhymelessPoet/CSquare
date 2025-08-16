#pragma once
#include <memory>

namespace CS
{
class SceneObject;
class ISystem;

class IComponent
{
public:
    friend class ISystem;
    using SystemType = ISystem;
    explicit IComponent(std::shared_ptr<SceneObject> owner);
    virtual ~IComponent();

    virtual void OnUpdate() = 0;

protected:
    std::shared_ptr<SceneObject> owner() const { return m_owner.lock(); }
    ISystem* system() const { return m_system; }

private:
    std::weak_ptr<SceneObject> m_owner;
    ISystem* m_system{nullptr};
};

class InvalidComponent : public IComponent
{
public:
    InvalidComponent() : IComponent(nullptr) {}
    InvalidComponent(const InvalidComponent&) = delete;
    InvalidComponent& operator=(const InvalidComponent&) = delete;
    InvalidComponent(InvalidComponent&&) = delete;
    InvalidComponent& operator=(InvalidComponent&&) = delete;

    virtual void OnUpdate() {};
    static InvalidComponent& Instance();
};

} // namespace CS
