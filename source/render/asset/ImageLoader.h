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
    static std::vector<std::byte> Load(const Path& imagePath, Size3u& imageSize, ImageFormat format);
};

} // namespace CS
