#include "PBRConfiguration.h"
#include "graphics/GraphicsPipeline.h"

namespace CS
{
void PBRConfiguration::Configure(GraphicsPipeline& pipeline) const
{
    pipeline.SetDepthTest(true);
    pipeline.SetDepthCompareOP(DepthCompareOp::Less);
}

} // namespace CS
