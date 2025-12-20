#pragma once

namespace CS
{

class GraphicsPipeline;

class IMaterialConfiguration
{
public:
    IMaterialConfiguration() = default;
    virtual ~IMaterialConfiguration() = default;

    virtual void Configure(GraphicsPipeline& pipeline) const = 0;
};

} // namespace CS
