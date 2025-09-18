#include "Sampler.h"
#include "GraphicsResourceDescriptors.h"

namespace CS
{

Sampler::Sampler(SamplerDescriptor* descriptor) : IGraphicsResource(descriptor) {}

void Sampler::SetAddressModeUV(AddressMode u, AddressMode v)
{
    descriptor<SamplerDescriptor>().SetAddressModeUV(u, v);
}

void Sampler::SetAddressModeW(AddressMode w)
{
    descriptor<SamplerDescriptor>().SetAddressModeW(w);
}

void Sampler::SetFilter(FilterMode min, FilterMode mag)
{
    descriptor<SamplerDescriptor>().SetFilter(min, mag);
}

void Sampler::SetMipmapFilter(MipmapFilterMode mode)
{
    descriptor<SamplerDescriptor>().SetMipmapFilter(mode);
}

} // namespace CS
