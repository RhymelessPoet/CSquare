#include "GraphicsResourceCache.h"
#include "IGraphicsResourceDescriptor.h"

namespace CS
{
GraphicsResourceCache::GraphicsResourceCache() {}

GraphicsResourceCache::~GraphicsResourceCache() {}

IGraphicsResourceDescriptor* GraphicsResourceCache::GetIDescriptor(size_t id) const
{
    auto itr = m_resources.find(id);
    if (itr != m_resources.end()) {
        return itr->second;
    }
    return nullptr;
}

bool GraphicsResourceCache::RemoveDescriptor(size_t id)
{
    auto itr = m_resources.find(id);
    if (itr != m_resources.end()) {
        itr->second->Destroy();
        delete itr->second;
        m_resources.erase(id);
        return true;
    }
    return false;
}

size_t GraphicsResourceCache::allocatedID()
{
    static size_t NextID = -1;
    return ++NextID;
}

} // namespace CS
