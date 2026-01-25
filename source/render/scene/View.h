#pragma once
#include "base/PImpl.h"
#include "graphics/RenderTarget.h"
#include <memory>

namespace CS
{
class ViewImpl;
class RenderContext;
class Scene;
class Camera;

using ViewID = uint32_t;
class View final : public PImpl<View>
{
private:
    struct ConstructorTag
    {
    };

public:
    friend class ViewGraph;

    View(ConstructorTag, ViewID id);
    View(ConstructorTag, ViewID id, RenderTarget target);
    ~View();

    void SetRenderTarget(RenderTarget target);
    RenderTarget GetRenderTarget();

    void SetScene(std::shared_ptr<Scene> scene);
    std::shared_ptr<Scene> GetScene() const;

    std::shared_ptr<Camera> GetCamera() const;

    void Render(RenderContext& context);

    ViewID GetID() const;

private:
    template <typename... Args>
    static std::shared_ptr<View> create(Args... args)
    {
        return std::make_shared<View>(ConstructorTag{}, std::forward<Args>(args)...);
    }
};

} // namespace CS
