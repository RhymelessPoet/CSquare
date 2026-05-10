#pragma once
#include "ViewType.h"
#include "base/Color.h"
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

    View(ConstructorTag, ViewID id, EViewType type);
    View(ConstructorTag, ViewID id, EViewType type, RenderTarget target);
    ~View();

    void SetRenderTarget(RenderTarget target);
    RenderTarget GetRenderTarget() const;

    void SetScene(std::shared_ptr<Scene> scene);
    std::shared_ptr<Scene> GetScene() const;

    std::shared_ptr<Camera> GetCamera() const;

    void OnRender(RenderContext& context);

    ViewID GetID() const;

    EViewType GetViewType() const;

    void SetCamera(std::shared_ptr<Camera> camera);

    void SetInput(uint32_t slotID, Texture texture);

    // Colour used to clear the colour attachment at the start of each frame.
    // Defaults to the editor-style dark grey (61, 61, 61).
    void SetClearColor(const Color& color);
    Color GetClearColor() const;

private:
    template <typename... Args>
    static std::shared_ptr<View> create(Args... args)
    {
        return std::make_shared<View>(ConstructorTag{}, std::forward<Args>(args)...);
    }
};

} // namespace CS
