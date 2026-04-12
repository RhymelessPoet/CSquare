#pragma once
#include "ImageFormat.h"
#include "base/FileSystem.h"
#include "base/Size.h"
#include <vector>

namespace CS
{

class ImageLoader
{
public:
    static std::vector<std::byte>
    Load(const Path& imagePath, Size3u& imageSize, ImageFormat& format, ImageFormat& internal);
    static std::vector<std::byte> LoadFloat(const Path& imagePath, Size3u& imageSize, ImageFormat format);

private:
    static bool loadImageToMemory(const Path& imagePath, std::vector<std::byte>& imageData);
};

} // namespace CS
