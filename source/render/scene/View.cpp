#include "View.h"
#include "Camera.h"
#include "Scene.h"
#include "graphics/GraphicsCommandBuffer.h"
#include "graphics/RenderTarget.h"
#include "renderer/RenderContext.h"
#include <optional>

namespace CS
{
template <>
struct ImplData<View>
{
    ImplData(ViewID id) : id(id) {}
    ImplData(ViewID id, RenderTarget target) : id(id), renderTarget(target) {}

    std::optional<RenderTarget> renderTarget;
    std::shared_ptr<Scene> scene;
    std::shared_ptr<Camera> camera;
    ViewID id;
};

View::View(ConstructorTag, ViewID id) : PImpl<View>(id) {}

View::View(ConstructorTag, ViewID id, RenderTarget target) : PImpl<View>(id, target) {}

View::~View() = default;

void View::SetRenderTarget(RenderTarget target)
{
    impl().renderTarget = target;
}

RenderTarget View::GetRenderTarget()
{
    return impl().renderTarget.value();
}

void View::SetScene(std::shared_ptr<Scene> scene)
{
    impl().scene = std::move(scene);
    impl().camera = impl().scene->CreateCamera();
}

std::shared_ptr<Scene> View::GetScene() const
{
    return impl().scene;
}

std::shared_ptr<Camera> View::GetCamera() const
{
    return impl().camera;
}

void View::Render(RenderContext& context)
{
    context.SetCamera(impl().camera);
    auto rtSize = GetRenderTarget().GetSize();
    auto cmdBuf = context.GetCommandBuffer();
    cmdBuf.BeginPass(GetRenderTarget())
        .Clear(Color(61.0f / 255.0f, 61.0f / 255.0f, 61.0f / 255.0f, 1.0f), 1.0f)
        .SetViewport(0, 0, rtSize.width, rtSize.height);
    impl().scene->OnRender(context);
    cmdBuf.EndPass();
}

ViewID View::GetID() const
{
    return impl().id;
}

} // namespace CS
