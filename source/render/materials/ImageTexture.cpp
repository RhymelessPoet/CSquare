#include "ImageTexture.h"

namespace CS
{

std::unique_ptr<MaterialTexture> ImageTexture::Clone() const
{
    return std::make_unique<ImageTexture>(*this);
}

bool ImageTexture::Equals(const MaterialTexture* other) const
{
    if (!MaterialTexture::Equals(other)) {
        return false;
    }
    const ImageTexture* otherImageTexture = dynamic_cast<const ImageTexture*>(other);
    if (otherImageTexture == nullptr) {
        return false;
    }
    return true;
}

void ImageTexture::SetImage(std::shared_ptr<Image> image)
{
    if (m_image != image) {
        m_image = std::move(image);
        m_sourceDirty = true;
    }
}

} // namespace CS
