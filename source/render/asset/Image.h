#pragma once
#include "ImageFormat.h"
#include "base/FileSystem.h"
#include "base/Size.h"
#include <vector>

namespace CS
{
class Image
{
public:
    Image(Path path, ImageFormat format = ImageFormat::RGBA8);
    ~Image();

    const Size3U& GetSize() const { return m_size; }
    size_t GetByteSize() const { return m_data.size(); }

    const std::byte* GetData() const;

    void Load(Path path = {}, ImageFormat format = ImageFormat::RGBA8);
    void Release();

private:
    ImageFormat m_format{ImageFormat::RGBA8};
    Path m_path;
    Size3U m_size;
    std::vector<std::byte> m_data;
};

} // namespace CS
