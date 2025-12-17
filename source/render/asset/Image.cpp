#include "Image.h"
#include "ImageLoader.h"

namespace CS
{

Image::Image(Path path, ImageFormat format) : m_path(std::move(path)), m_format(format)
{
    m_data = ImageLoader::Load(m_path, m_size, m_format);
}

Image::Image(std::vector<std::byte> data, ImageFormat format) : m_data(std::move(data)), m_format(format) {}

Image::~Image() {}

void Image::Load(Path path, ImageFormat format)
{
    m_format = format;
    if (!path.empty()) {
        m_path = std::move(path);
    }

    m_data = ImageLoader::Load(m_path, m_size, m_format);
}

const std::byte* Image::GetData() const
{
    return m_data.data();
}

const std::span<const std::byte> Image::GetDataView() const
{
    return std::span<const std::byte>(m_data);
}

} // namespace CS
