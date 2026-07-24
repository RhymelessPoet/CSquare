#include "RenderTexture.h"

namespace CS
{
std::unique_ptr<MaterialTexture> RenderTexture::Clone() const
{
    return std::make_unique<RenderTexture>(*this);
}

} // namespace CS
