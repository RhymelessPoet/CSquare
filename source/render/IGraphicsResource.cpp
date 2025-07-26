#include "IGraphicsResource.h"
#include "IGraphicsResourceDescriptor.h"
#include "graphics/GraphicsResourceCache.h"

namespace CS
{

IGraphicsResource::IGraphicsResource(IGraphicsResourceDescriptor* descriptor) noexcept : m_descriptor(descriptor)
{
    m_descriptor->AddReference();
}

IGraphicsResource::~IGraphicsResource()
{
    m_descriptor->Release();
}

IGraphicsResource::IGraphicsResource(const IGraphicsResource& other)
{
    m_descriptor = other.m_descriptor;
    m_descriptor->AddReference();
}

IGraphicsResource& IGraphicsResource::operator=(const IGraphicsResource& other)
{
    if (this != &other) {
        m_descriptor = other.m_descriptor;
        m_descriptor->AddReference();
    }
    return *this;
}

IGraphicsResource::IGraphicsResource(IGraphicsResource&& other) noexcept
{
    std::swap(m_descriptor, other.m_descriptor);
}

IGraphicsResource& IGraphicsResource::operator=(IGraphicsResource&& other) noexcept
{
    if (this != &other) {
        m_descriptor = other.m_descriptor;
        other.m_descriptor = nullptr;
    }
    return *this;
}

bool operator==(const IGraphicsResource& lft, const IGraphicsResource& rhs)
{
    return lft.GetID() == rhs.GetID();
}

size_t IGraphicsResource::GetID() const
{
    return m_descriptor->GetID();
}

bool IGraphicsResource::IsBuild() const
{
    return m_descriptor->IsBuild();
}

bool IGraphicsResource::Build()
{
    return m_descriptor->Build();
}

} // namespace CS
