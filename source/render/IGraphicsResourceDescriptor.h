#pragma once
#include <memory>

namespace CS
{
class GraphicsGLImpl;
class GraphicsResourceCache;

class IGraphicsResourceDescriptor
{
public:
    IGraphicsResourceDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI);
    virtual ~IGraphicsResourceDescriptor() = 0 {};

    void AddReference();
    void Release();

    virtual bool IsBuild() const = 0;
    bool Build();
    virtual void Destroy() = 0;

    virtual bool IsDirty() const { return m_isDirty; }

    size_t GetID() const { return m_id; }
    std::shared_ptr<GraphicsGLImpl> GetGraphicsAPI() const;

protected:
    virtual bool build() = 0;
    inline void setDirty() { m_isDirty = true; }

private:
    inline void resetDirty() { m_isDirty = false; }

protected:
    std::weak_ptr<GraphicsGLImpl> m_graphicsAPI;
    std::shared_ptr<GraphicsResourceCache> m_cache;

private:
    bool m_isDirty{false};
    size_t m_id{std::numeric_limits<size_t>::max()};
    size_t m_refCount{0u};
};

} // namespace CS
