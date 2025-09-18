#include "MaterialTexture.h"

namespace CS
{
MaterialTexture::MaterialTexture() {}

MaterialTexture::MaterialTexture(AddressMode u, AddressMode v, FilterMode minFilter, FilterMode magFilter)
    : m_u(u), m_v(v), m_minFilter(minFilter), m_magFilter(magFilter)
{}

MaterialTexture::~MaterialTexture() = default;

bool MaterialTexture::Equals(const MaterialTexture* other) const
{
    if (other == nullptr) {
        return false;
    }
    if (this == other) {
        return true;
    }
    if (m_u == other->m_u && m_v == other->m_v && m_minFilter == other->m_minFilter &&
        m_magFilter == other->m_magFilter && m_mipmapFilter == other->m_mipmapFilter &&
        m_useMipmaps == other->m_useMipmaps && m_isSRGB == other->m_isSRGB)
    {
        return true;
    }
    return false;
}

void MaterialTexture::SetAddressModeUV(AddressMode u, AddressMode v)
{
    if (m_u != u || m_v != v) {
        m_u = u;
        m_v = v;
        m_dirty = true;
    }
}

void MaterialTexture::SetFilter(FilterMode min, FilterMode mag)
{
    if (m_minFilter != min || m_magFilter != mag) {
        m_minFilter = min;
        m_magFilter = mag;
        m_dirty = true;
    }
}

void MaterialTexture::SetMipmapFilter(MipmapFilterMode mode)
{
    if (m_mipmapFilter != mode) {
        m_mipmapFilter = mode;
        m_dirty = true;
    }
}

} // namespace CS
