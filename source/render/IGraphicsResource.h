#pragma once
#include <cstdint>
#include <memory>
#include <numeric>

namespace CS
{
class IGraphicsResourceDescriptor;
class GraphicsResourceCache;

class IGraphicsResource
{
public:
    virtual ~IGraphicsResource() = 0;

    IGraphicsResource(const IGraphicsResource& other);
    IGraphicsResource& operator=(const IGraphicsResource& other);
    IGraphicsResource(IGraphicsResource&& other) noexcept;
    IGraphicsResource& operator=(IGraphicsResource&& other) noexcept;

    friend bool operator==(const IGraphicsResource& lft, const IGraphicsResource& rhs);

    size_t GetID() const;
    bool IsBuild() const;
    bool Build();

protected:
    IGraphicsResource(IGraphicsResourceDescriptor* descriptor) noexcept;

    template <typename DescriptorType>
    const DescriptorType& descriptor() const
    {
        DescriptorType* _descriptor = dynamic_cast<DescriptorType*>(m_descriptor);
        return *_descriptor;
    }

    template <typename DescriptorType>
    DescriptorType& descriptor()
    {
        DescriptorType* _descriptor = dynamic_cast<DescriptorType*>(m_descriptor);
        return *_descriptor;
    }

private:
    IGraphicsResourceDescriptor* m_descriptor{nullptr};
};

} // namespace CS
