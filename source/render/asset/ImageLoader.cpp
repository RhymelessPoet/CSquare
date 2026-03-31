#include "ImageLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <format>
#include <fstream>
#include <iostream>

namespace CS
{
std::vector<std::byte> ImageLoader::Load(const Path& imagePath, Size3u& imageSize, ImageFormat& format)
{
    auto absolutePath = imagePath;
    if (!imagePath.is_absolute()) {
        absolutePath = FileSystem::absolute(imagePath);
    }

    if (!FileSystem::exists(absolutePath)) {
        // TODO: log error
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
        // TODO: log error
        return std::vector<std::byte>();
    }

    std::cerr << std::format("Load Image {}, Size({}, {}, {}) \n", imagePath.string(), width, height, channels);

    int requiredChannels = channels;
    if (channels == 1) {
        requiredChannels = 3;       // Treat single channel images as RGB
        format = ImageFormat::RGB8; // Default to RGB8 for single channel images
    } else {
        imageSize = {width, height, requiredChannels};
        format = (requiredChannels == 4) ? ImageFormat::RGBA8 : ImageFormat::RGB8;
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
        // TODO: log error
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
