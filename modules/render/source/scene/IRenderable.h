#pragma once
#include "IComponent.h"

namespace CS
{

class RenderContext;
class GraphicsAPI;
class Transform;

class IRenderable : public IComponent
{
public:
    IRenderable(std::shared_ptr<SceneObject> owner) : IComponent(std::move(owner)) {}
    ~IRenderable() = default;

    virtual void OnRender(RenderContext& context) = 0;

protected:
    const Transform& transform() const;
    Transform& transform();
};

} // namespace CS
