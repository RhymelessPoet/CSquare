#pragma once
#include "graphics/MaterialTexture.h"

namespace CS
{
class ImageTexture : public MaterialTexture
{
public:
    ImageTexture() = default;
    explicit ImageTexture(std::shared_ptr<Image> image) : m_image(std::move(image)) {}
    ImageTexture(std::shared_ptr<Image> image,
                 AddressMode u,
                 AddressMode v,
                 FilterMode minFilter = FilterMode::Linear,
                 FilterMode magFilter = FilterMode::Linear)
        : MaterialTexture(u, v, minFilter, magFilter), m_image(std::move(image))
    {}
    CS_DEFAULT_COPY_MOVE(ImageTexture);

    virtual std::unique_ptr<MaterialTexture> Clone() const override;

    virtual bool Equals(const MaterialTexture* other) const override;

    void SetImage(std::shared_ptr<Image> image);
    std::shared_ptr<Image> GetImage() const { return m_image; }

    bool IsSourceChanged() const { return m_sourceDirty; }
    void ResetSourceChange() { m_sourceDirty = false; }

private:
    bool m_sourceDirty : 1 {true};
    std::shared_ptr<Image> m_image;
};

} // namespace CS
