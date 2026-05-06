#include "ImageLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "base/Logger.h"
#include "stb/stb_image.h"
#include <format>
#include <fstream>

namespace CS
{
std::vector<std::byte>
ImageLoader::Load(const Path& imagePath, Size3u& imageSize, ImageFormat& format, ImageFormat& internal)
{
    auto absolutePath = imagePath;
    if (!imagePath.is_absolute()) {
        absolutePath = FileSystem::absolute(imagePath);
    }

    if (!FileSystem::exists(absolutePath)) {
        CS::LogError(::CS::BuiltInChannels::Asset(), CS::Fmt("Image file not found: {}", absolutePath.string()));
        return std::vector<std::byte>();
    }

    int width, height, channels;
    std::vector<std::byte> imageMemory;
    std::vector<std::byte> imageData;

    stbi_set_flip_vertically_on_load(true);

    loadImageToMemory(absolutePath, imageMemory);
    bool infoSuccess = stbi_info_from_memory(reinterpret_cast<stbi_uc*>(imageMemory.data()),
                                             static_cast<int>(imageMemory.size()), &width, &height, &channels);
    if (!infoSuccess) {
        CS::LogError(::CS::BuiltInChannels::Asset(), CS::Fmt("Failed to read image info: {}", absolutePath.string()));
        return std::vector<std::byte>();
    }

    CS::LogInfo(::CS::BuiltInChannels::Asset(), CS::Fmt("Load Image {}, Size({}, {}, {})", imagePath.string(), width, height,
                channels));

    int requiredChannels = channels;
    if (channels == 1) {
        requiredChannels = 3;       // Treat single channel images as RGB
        format = ImageFormat::RGB8; // Default to RGB8 for single channel images
        internal = ImageFormat::R8;
    } else {
        imageSize = {width, height, requiredChannels};
        format = (requiredChannels == 4) ? ImageFormat::RGBA8 : ImageFormat::RGB8;
        internal = format;
    }

    auto imageMemoryBuffer = reinterpret_cast<stbi_uc*>(imageMemory.data());
    auto _data = stbi_load_from_memory(imageMemoryBuffer, static_cast<int>(imageMemory.size()), &width, &height,
                                       &channels, requiredChannels);
    std::byte* data = reinterpret_cast<std::byte*>(_data);

    if (data != nullptr) {
        imageSize = {width, height, requiredChannels};
        auto imageByteSize = width * height * requiredChannels * ChannelByteSize(ChannelType(format));
        imageData.resize(imageByteSize);
        std::copy(data, data + imageByteSize, imageData.begin());
        stbi_image_free(data);
    }

    return imageData;
}

std::vector<std::byte> ImageLoader::LoadFloat(const Path& imagePath, Size3u& imageSize, ImageFormat format)
{
    auto absolutePath = imagePath;
    if (!imagePath.is_absolute()) {
        absolutePath = FileSystem::absolute(imagePath);
    }

    if (!FileSystem::exists(absolutePath)) {
        CS::LogError(::CS::BuiltInChannels::Asset(), CS::Fmt("Image (float) file not found: {}", absolutePath.string()));
        return std::vector<std::byte>();
    }
    int width, height, channels;
    std::vector<std::byte> imageData;

    float* _data = stbi_loadf(absolutePath.string().c_str(), &width, &height, &channels, Channels(format));
    std::byte* data = reinterpret_cast<std::byte*>(_data);

    if (data != nullptr) {
        imageSize = {width, height, channels};
        auto imageByteSize = width * height * channels * ChannelByteSize(ChannelType(format));
        imageData.resize(imageByteSize);
        std::copy(data, data + imageByteSize, imageData.begin());
        stbi_image_free(data);
    }

    return imageData;
}

bool ImageLoader::loadImageToMemory(const Path& imagePath, std::vector<std::byte>& imageData)
{
    // Implementation for loading image to memory
    std::ifstream file(imagePath, std::ifstream::in | std::ios::binary | std::ios::ate);
    if (!file) {
        return false;
    }

    imageData.reserve((size_t)file.tellg());
    file.seekg(0, std::ios::beg);
    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    imageData.assign(reinterpret_cast<std::byte*>(buffer.data()),
                     reinterpret_cast<std::byte*>(buffer.data() + buffer.size()));

    return true;
}

} // namespace CS