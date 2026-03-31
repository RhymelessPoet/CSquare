#pragma once
#include "base/Macros.h"
#include "graphics/GraphicsResourceParameters.h"
#include <memory>

namespace CS
{
class Image;

class MaterialTexture
{
public:
    MaterialTexture(/* args */);
    MaterialTexture(AddressMode u,
                    AddressMode v,
                    FilterMode minFilter = FilterMode::Linear,
                    FilterMode magFilter = FilterMode::Linear);

    virtual ~MaterialTexture();

    CS_DEFAULT_COPY_MOVE(MaterialTexture);

    virtual std::unique_ptr<MaterialTexture> Clone() const = 0;

    virtual bool Equals(const MaterialTexture* other) const;

    void SetAddressModeUV(AddressMode u, AddressMode v);
    std::pair<AddressMode, AddressMode> GetAddressModeUV() const { return {m_u, m_v}; }

    void SetFilter(FilterMode min, FilterMode mag);
    std::pair<FilterMode, FilterMode> GetFilter() const { return {m_minFilter, m_magFilter}; }

    void SetMipmapFilter(MipmapFilterMode mode);
    MipmapFilterMode GetMipmapFilter() const { return m_mipmapFilter; }

    void SetSRGB(bool isSRGB) { m_isSRGB = isSRGB; }
    bool IsSRGB() const { return m_isSRGB; }

    void SetUseMipmaps(bool useMipmaps) { m_useMipmaps = useMipmaps; }
    bool UseMipmaps() const { return m_useMipmaps; }

    bool IsDirty() const { return m_dirty; }
    void ResetDirty() { m_dirty = false; }

private:
    AddressMode m_u{AddressMode::Repeat};
    AddressMode m_v{AddressMode::Repeat};
    FilterMode m_minFilter{FilterMode::Linear};
    FilterMode m_magFilter{FilterMode::Linear};
    MipmapFilterMode m_mipmapFilter{MipmapFilterMode::Max};
    bool m_useMipmaps : 1 {false};
    bool m_isSRGB : 1 {false};
    bool m_dirty : 1 {true};
};

inline bool operator==(const std::unique_ptr<const MaterialTexture>& lhs,
                       const std::unique_ptr<const MaterialTexture>& rhs)
{
    return lhs->Equals(rhs.get());
}

inline bool operator!=(const std::unique_ptr<const MaterialTexture>& lhs,
                       const std::unique_ptr<const MaterialTexture>& rhs)
{
    return !(lhs == rhs);
};

} // namespace CS
