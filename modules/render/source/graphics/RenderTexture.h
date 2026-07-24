#pragma once
#include "MaterialTexture.h"

namespace CS
{

class RenderTexture : public MaterialTexture
{
public:
    RenderTexture() = default;
    RenderTexture(AddressMode u,
                  AddressMode v,
                  FilterMode minFilter = FilterMode::Linear,
                  FilterMode magFilter = FilterMode::Linear)
        : MaterialTexture(u, v, minFilter, magFilter)
    {}

    CS_DEFAULT_COPY_MOVE(RenderTexture);

    virtual std::unique_ptr<MaterialTexture> Clone() const override;
};

} // namespace CS
