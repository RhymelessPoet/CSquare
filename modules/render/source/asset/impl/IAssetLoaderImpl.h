#pragma once
#include "FileSystem.h"
#include <memory>

namespace CS
{

class AssetScene;

class IAssetLoaderImpl
{
public:
    IAssetLoaderImpl(/* args */) = default;
    virtual ~IAssetLoaderImpl() {}

    virtual std::shared_ptr<AssetScene> Load(const Path& path) = 0;
};

} // namespace CS
