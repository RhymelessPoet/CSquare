#include "IMaterialConfiguration.h"

namespace CS
{

void IMaterialConfiguration::SetMaterialID(uint16_t id)
{
    for (auto& slot : m_requisiteSlots) {
        slot.id.materialID = id;
    }
    for (auto& slot : m_targetSlots) {
        slot.id.materialID = id;
    }
}

bool IMaterialConfiguration::ConnectFrom(const SlotID& from, const SlotID& to)
{
    for (auto& slot : m_targetSlots) {
        if (from == slot.id) {
            slot.connectedSlot = to;
            return true;
        }
    }
    return false;
}

bool IMaterialConfiguration::ConnectTo(const SlotID& from, const SlotID& to)
{
    for (auto& slot : m_requisiteSlots) {
        if (to == slot.id) {
            slot.connectedSlot = from;
            return true;
        }
    }
    return false;
}

bool IMaterialConfiguration::DisconnectFrom(const SlotID& id)
{
    for (auto& slot : m_targetSlots) {
        if (id == slot.id) {
            slot.connectedSlot.reset();
            return true;
        }
    }
    return false;
}

bool IMaterialConfiguration::DisconnectTo(const SlotID& id)
{
    for (auto& slot : m_requisiteSlots) {
        if (id == slot.id) {
            slot.connectedSlot.reset();
            return true;
        }
    }
    return false;
}

bool IMaterialConfiguration::Bind(const SlotID& id, uint32_t binding)
{
    for (auto& slot : m_requisiteSlots) {
        if (id == slot.id) {
            slot.binding = binding;
            return true;
        }
    }
    return false;
}

std::optional<SlotID> IMaterialConfiguration::GetSlot(uint32_t binding) const
{
    for (auto& slot : m_requisiteSlots) {
        if (slot.binding.has_value() && slot.binding.value() == binding) {
            return slot.id;
        }
    }
    return std::nullopt;
}

} // namespace CS
