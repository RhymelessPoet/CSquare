#pragma once
#include "base/ExtensibleEnum.h"
#include <bitset>
#include <memory>

namespace CS
{
class SceneObject;
class ISystem;
class SystemContext;

struct ComponentState
{
    using UnderlyingType = uint8_t;
    static constexpr size_t NameLength{16u};
    static constexpr size_t MaxCount = 32u;
};

const inline EnumClass<ComponentState, "New", "Active", "Destroyed"> BaseComponentStates;

using EComponentState = Enum<ComponentState>;
using EComponentStateFlags = std::bitset<ComponentState::MaxCount>;

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

    void setState(EComponentState state, bool value);
    bool isOn(EComponentState state) const;

private:
    EComponentStateFlags m_stateFlags;
    std::weak_ptr<SceneObject> m_owner;
    ISystem* m_system{nullptr};
};

} // namespace CS
