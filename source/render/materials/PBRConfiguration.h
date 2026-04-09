#pragma once
#include "IMaterialConfiguration.h"

namespace CS
{

class PBRConfiguration : public IMaterialConfiguration
{
public:
    void Configure(GraphicsPipeline& pipeline) const override;
};

} // namespace CS
