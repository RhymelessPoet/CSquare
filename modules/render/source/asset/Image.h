#pragma once
#include "ImageFormat.h"
#include "FileSystem.h"
#include "Size.h"
#include <span>
#include <vector>

namespace CS
{
class Image
{
public:
    explicit Image(Path path, ImageFormat format = ImageFormat::RGBA8);
    explicit Image(std::vector<std::byte> data, ImageFormat format = ImageFormat::RGBA8);
    ~Image();

    const Size3u& GetSize() const { return m_size; }
    size_t GetByteSize() const { return m_data.size(); }

    const std::byte* GetData() const;
    const std::span<const std::byte> GetDataView() const;

    void Load(Path path = {}, ImageFormat format = ImageFormat::RGBA8);
    void Release();

    ImageFormat GetFormat() const { return m_format; }
    ImageFormat GetInternalFormat() const { return m_internalFormat; }

    const Path& GetPath() const { return m_path; }

private:
    std::string m_name;
    ImageFormat m_format{ImageFormat::RGBA8};
    ImageFormat m_internalFormat{ImageFormat::RGBA8};
    Path m_path;
    Size3u m_size;
    std::vector<std::byte> m_data;
};

} // namespace CS
