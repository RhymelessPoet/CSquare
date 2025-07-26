#pragma once
#include <unordered_map>

namespace CS
{
class IGraphicsResourceDescriptor;
class GraphicsAPI;
class GraphicsResourceCache
{
public:
    GraphicsResourceCache(/* args */);
    ~GraphicsResourceCache();

    template <typename DescriptorType, typename... Args>
    size_t Allocate(Args&&... args)
    {
        size_t id = allocatedID();
        m_resources[id] = new DescriptorType(id, std::forward<Args>(args)...);
        return id;
    }

    template <typename DescriptorType>
    DescriptorType* GetDescriptor(size_t id) const
    {
        return dynamic_cast<DescriptorType*>(GetIDescriptor(id));
    }

    IGraphicsResourceDescriptor* GetIDescriptor(size_t id) const;

    bool RemoveDescriptor(size_t id);

private:
    static size_t allocatedID();

private:
    std::unordered_map<size_t, IGraphicsResourceDescriptor*> m_resources;
};

} // namespace CS
