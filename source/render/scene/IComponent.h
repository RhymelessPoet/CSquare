#pragma once
#include <memory>

namespace CS
{
class SceneObject;
class ISystem;
class SystemContext;

class IComponent
{
public:
    friend class ISystem;
    using SystemType = ISystem;
    explicit IComponent(std::shared_ptr<SceneObject> owner);
    virtual ~IComponent();

    virtual void OnUpdate(SystemContext& context) = 0;

    virtual bool IsValid() const { return true; }

    std::shared_ptr<SceneObject> GetSO() const { return owner(); }

protected:
    std::shared_ptr<SceneObject> owner() const { return m_owner.lock(); }
    std::shared_ptr<SceneObject> owner() { return m_owner.lock(); }

    ISystem* system() const { return m_system; }

private:
    std::weak_ptr<SceneObject> m_owner;
    ISystem* m_system{nullptr};
};

} // namespace CS
