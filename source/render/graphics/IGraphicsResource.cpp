#include "IGraphicsResource.h"
#include "IGraphicsResourceDescriptor.h"
#include "graphics/GraphicsResourceCache.h"

namespace CS
{

IGraphicsResource::IGraphicsResource(IGraphicsResourceDescriptor* descriptor) noexcept : m_descriptor(descriptor)
{
    if (IsValid()) {
        m_descriptor->AddReference();
    }
}

IGraphicsResource::~IGraphicsResource()
{
    if (IsValid()) {
        m_descriptor->Release();
    }
}

IGraphicsResource::IGraphicsResource(const IGraphicsResource& other)
{
    m_descriptor = other.m_descriptor;

    if (m_descriptor != nullptr) {
        m_descriptor->AddReference();
    }
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
    if (!IsValid()) {
        return std::numeric_limits<size_t>::max();
    }
    return m_descriptor->GetID();
}

bool IGraphicsResource::IsBuild() const
{
    if (!IsValid()) {
        return false;
    }
    return m_descriptor->IsBuild();
}

bool IGraphicsResource::Build()
{
    if (!IsValid()) {
        return false;
    }
    return m_descriptor->Build();
}

} // namespace CS
