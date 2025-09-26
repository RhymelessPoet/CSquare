#include "ImageLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace CS
{
std::vector<std::byte> ImageLoader::Load(const Path& imagePath, Size3U& imageSize, ImageFormat format)
{
    auto absolutePath = imagePath;
    if (!imagePath.is_absolute()) {
        absolutePath = FileSystem::absolute(imagePath);
    }

    if (!FileSystem::exists(absolutePath)) {
        // TODO: log error
        return std::vector<std::byte>();
    }

    int x, y, z;
    std::byte* data = nullptr;

    if (format == ImageFormat::RGB8 || format == ImageFormat::RGBA8) {
        auto _data = stbi_load(absolutePath.string().c_str(), &x, &y, &z, Channels(format));
        data = reinterpret_cast<std::byte*>(_data);
    }
    if (format == ImageFormat::RGB32Float || format == ImageFormat::RGBA32Float) {
        float* _data = stbi_loadf(absolutePath.string().c_str(), &x, &y, &z, Channels(format));
        data = reinterpret_cast<std::byte*>(_data);
    }

    std::vector<std::byte> imageData;

    if (data != nullptr) {
        imageSize = {x, y, z};
        auto imageByteSize = x * y * z * ChannelByteSize(ChannelType(format));
        imageData.resize(imageByteSize);
        std::copy(data, data + imageByteSize, imageData.begin());
        stbi_image_free(data);
    }

    return imageData;
}

} // namespace CS
