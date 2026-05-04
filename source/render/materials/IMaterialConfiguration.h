#pragma once
#include "SlotDescription.h"
#include "scene/ViewType.h"

#include <vector>
namespace CS
{

class GraphicsPipeline;

class IMaterialConfiguration
{
public:
    IMaterialConfiguration() = default;
    virtual ~IMaterialConfiguration() = default;

    virtual void Configure(GraphicsPipeline& pipeline) const = 0;
    std::span<const SlotDescription> GetRequisiteSlots() const { return m_requisiteSlots; };
    std::span<const SlotDescription> GetTargetSlots() const { return m_targetSlots; };

    EViewType GetViewType() const { return m_viewType; }
    std::optional<Size2u> GetTargetSize() const { return m_targetSize; }

    void SetMaterialID(uint16_t id);

    bool ConnectFrom(const SlotID& from, const SlotID& to);
    bool ConnectTo(const SlotID& from, const SlotID& to);
    bool DisconnectFrom(const SlotID& id);
    bool DisconnectTo(const SlotID& id);

    bool Bind(const SlotID& id, uint32_t binding);
    std::optional<SlotID> GetSlot(uint32_t binding) const;

protected:
    std::optional<Size2u> m_targetSize;
    std::vector<SlotDescription> m_requisiteSlots;
    std::vector<SlotDescription> m_targetSlots;
    EViewType m_viewType;
};

} // namespace CS
