#include "PBRConfiguration.h"
#include "graphics/GraphicsPipeline.h"

namespace CS
{
PBRConfiguration::PBRConfiguration()
{
    m_viewType = EViewType::Make<"3D_Main">();
    m_requisiteSlots.push_back(SlotDescription{.id = {.slotIndex = 0u}, .type = SlotType::Depth});

    m_targetSlots.push_back(SlotDescription{.id = {.slotIndex = 1u}, .type = SlotType::Color});
    m_targetSlots.push_back(SlotDescription{.id = {.slotIndex = 2u}, .type = SlotType::DepthStencil});
}

void PBRConfiguration::Configure(GraphicsPipeline& pipeline) const
{
    pipeline.SetDepthTest(true);
    pipeline.SetDepthCompareOP(DepthCompareOp::Less);
}

} // namespace CS
