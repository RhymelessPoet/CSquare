#include "IGraphicsResourceDescriptor.h"
#include "graphics/GraphicsResourceCache.h"
#include "graphics/opengl/GraphicsGLImpl.h"

namespace CS
{
IGraphicsResourceDescriptor::IGraphicsResourceDescriptor(size_t id, std::shared_ptr<GraphicsGLImpl> graphicsAPI)
    : m_id(id), m_graphicsAPI(graphicsAPI)
{
    m_cache = graphicsAPI->GetResourceCache();
}

void IGraphicsResourceDescriptor::AddReference()
{
    ++m_refCount;
}

void IGraphicsResourceDescriptor::Release()
{
    if (m_refCount != 0u) {
        --m_refCount;
        if (m_refCount == 0u) {
            m_cache->RemoveDescriptor(m_id);
        }
    } else {
        m_cache->RemoveDescriptor(m_id);
    }
}

std::shared_ptr<GraphicsGLImpl> IGraphicsResourceDescriptor::GetGraphicsAPI() const
{
    return m_graphicsAPI.lock();
}

bool IGraphicsResourceDescriptor::Build()
{
    bool isNeedBuild = !IsBuild() || m_isDirty;
    bool isNeedDestroy = IsBuild() && m_isDirty;

    if (isNeedDestroy) {
        Destroy();
    }

    if (isNeedBuild) {
        if (build()) {
            resetDirty();
            return true;
        } else {
            return false;
        }
    } else {
        return true;
    }
}

} // namespace CS
