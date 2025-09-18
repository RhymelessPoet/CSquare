#pragma once
#include "IGraphicsResource.h"
#include "SamplerParameters.h"

namespace CS
{

class SamplerDescriptor;

class Sampler final : public IGraphicsResource
{
public:
    friend class GraphicsAPI;
    Sampler(/* args */) = default;
    ~Sampler() = default;

    void SetAddressModeUV(AddressMode u, AddressMode v);
    void SetAddressModeW(AddressMode w);

    void SetFilter(FilterMode min, FilterMode mag);
    void SetMipmapFilter(MipmapFilterMode mode);

private:
    Sampler(SamplerDescriptor* descriptor);
};

} // namespace CS
