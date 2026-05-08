# Render Presentation Strategy

## 1. Purpose

This document describes how the CSquare editor presents the engine's 3D output
inside a Qt Quick UI. It consolidates the design in the
`Render_Presentation_Refactor_362bd263` plan together with the follow-up
stabilization work that turned the design into a reliable startup pipeline.

The strategy addresses four concerns simultaneously:

1. **Native OpenGL context ownership** — the engine owns its own `HGLRC` on
   the `QQuickWindow`'s native `HWND` and shares resources with Qt's RHI
   context via `wglShareLists`.
2. **Single-threaded frame loop** — `RenderModule::Update` runs ECS and
   `Render` sequentially on the main thread; `QSG_RENDER_LOOP=basic` keeps
   Qt's scene-graph on the same thread.
3. **Stable Qt Quick integration** — the render view is a plain `QQuickItem`
   whose `updatePaintNode` samples an engine-owned GL color texture via
   `QSGSimpleTextureNode` + `QNativeInterface::QSGOpenGLTexture::fromNative`.
4. **Deterministic startup ordering** — QML load, engine renderer creation,
   scene initialization, asset loading, and camera fit happen in a strict
   sequence controlled by Qt signals, never inside native-event flushes.

## 2. Architectural Context

### 2.1 Threading model

| Thread         | Responsibilities                                              |
|----------------|---------------------------------------------------------------|
| Main thread    | `Engine::Run` loop, ECS update, engine render to FBO, Qt scene graph, QML event loop |
| Asset worker   | Background `std::async` GLTF parsing (via `AssetManager::GetAssetSceneAsync`) |

All OpenGL work runs on the main thread. `wglMakeCurrent` is only toggled
between Qt's HGLRC and the engine's HGLRC on this single thread.

### 2.2 OpenGL context topology

```
HWND (QQuickWindow::winId())
  ├─ Qt RHI HGLRC  (owned by Qt QSG)  ← wglShareLists ───┐
  └─ Engine HGLRC  (owned by WGLWindowContext)  ─────────┘
        │
        └─ Engine FBO (color + depth textures, RGBA8/Depth32)
              │
              └─ GL texture name surfaced to QSG via
                 QNativeInterface::QSGOpenGLTexture::fromNative
```

`wglShareLists(qtHGLRC, engineHGLRC)` is called **before any object is
created on either context** so the engine's FBO color texture is visible to
Qt for sampling in `QSGSimpleTextureNode`.

### 2.3 Module topology

- `RenderModule` (engine) — owns `Renderer`, `ViewGraph`, `GraphicsAPI`,
  and the `SystemGraph` (MeshRender/Light/Camera/Transform systems).
- `QUIModule` (editor) — drives Qt; sets `QSG_RENDER_LOOP=basic`; pumps the
  Qt event loop each tick.
- `UIApplication` (editor) — the `QGuiApplication` subclass; registers QML
  types and defers `QQmlApplicationEngine::load` via `QTimer::singleShot(0,…)`.
- `QuickRenderView` (editor) — the `QQuickItem` that bridges engine output
  into the QML scene graph and forwards user input back to the engine.
- `QEditor` (editor) — the QML-facing controller that creates `ProjectModel`
  instances, updates tree models, and coordinates asset loading.

## 3. Key Files

| File | Role |
|------|------|
| `source/render/graphics/opengl/NativeContext.{h,cpp}` | `WGLWindowContext : INativeContext` — native HWND/HDC/HGLRC wiring and `wglShareLists`. |
| `source/render/graphics/GLRendererBuilder.{h,cpp}` | `SetNativeWindow(void* hwnd, void* sharedHGLRC)` branch that picks `WGLWindowContext` over the default GLFW path. |
| `source/render/RenderModule.{h,cpp}` | Single-thread `Update` (ECS + Render). No mutex/cv. |
| `source/editor/QuickRenderView.{h,cpp}` | `QQuickItem`-based view, deferred engine init, QSG texture sharing, input handling. |
| `source/editor/QEditor.{h,cpp}` | `Q_INVOKABLE loadProject / beginAssetLoading`; emits `sceneAssetLoaded`. |
| `source/editor/ProjectModel.{h,cpp}` | `initializeScene()` (sky + directional light + hierarchy), `startAssetLoad()` (async GLTF), `pollAsyncLoad()`. |
| `source/editor/ProjectManager.{h,cpp}` | `AttachAllToView(view)` — binds pending scenes to the main View after renderer is ready. |
| `source/editor/UIApplication.{h,cpp}` | QML engine holder, deferred `load()` via `QTimer::singleShot`. |
| `source/editor/qml/CSAppWindow.qml` | Layout + signal wiring (`onEngineReady`, `onSceneAssetLoaded`). |
| `source/render/CMakeLists.txt` | `opengl32 gdi32 user32` link on Windows (resolves `wgl*`). |

Files removed by the refactor: `QuickRenderer.{h,cpp}`, `QBlitRenderPass.{h,cpp}`.

## 4. Qt Signals & Events Orchestrating Startup

The pipeline is intentionally signal-driven. Every phase transitions on an
asynchronous event so that no heavy work is performed inside a Qt synchronous
signal frame.

| Emitter | Signal / hook | Handler | Purpose |
|---------|---------------|---------|---------|
| `UIApplication` ctor | `QTimer::singleShot(0,…)` | lambda → `m_qmlEngine.load(CSAppWindow.qml)` | Defer QML load until the event loop is running. |
| `QQuickWindow` | `sceneGraphInitialized` (Qt::QueuedConnection) | `QuickRenderView::initializeEngineRenderer` | Break out of Qt's scene-graph signal frame before doing GL work. |
| `QuickRenderView` | `engineReady()` | QML `onEngineReady: CSEditor.beginAssetLoading()` | Asset loading only starts after the engine's main `View` exists. |
| `QTimer` (100 ms) | `timeout` | `ProjectModel::pollAsyncLoad` | Poll the async GLTF future on the main thread. |
| `ProjectModel` | `setOnAssetLoaded` callback | `QEditor` lambda: `updateModels` + `emit sceneAssetLoaded()` | Refresh QML tree and notify listeners. |
| `QEditor` | `sceneAssetLoaded()` | QML `onSceneAssetLoaded: renderView.FitToScene(true)` | Fit camera to the now-populated scene AABB. |

### Key connection rule

`sceneGraphInitialized → initializeEngineRenderer` **must** use
`Qt::QueuedConnection`. A `DirectConnection` runs GL work inside Qt's
synchronous signal frame, which reenters
`flushWindowSystemEvents → sendEvent → notify` recursively and overflows the
stack under `QSG_RENDER_LOOP=basic`.

## 5. Initialization Flow

```
main
 └─ Engine::Run loop spins up modules
     ├─ RenderModule (no renderer yet: WGL context needs an HWND)
     └─ QUIModule sets QSG_RENDER_LOOP=basic and constructs UIApplication
         └─ UIApplication ctor:
             ├─ register QML types (CSQuickRenderView, CSTheme, …)
             └─ QTimer::singleShot(0, load CSAppWindow.qml)      [A]

[A] QML load runs on the event loop
 ├─ ApplicationWindow instantiated
 ├─ CSQuickRenderView ctor
 │    ├─ setFlag(ItemHasContents); accept mouse + hover
 │    └─ itemChange(ItemSceneChange):
 │        connect(sceneGraphInitialized, Qt::QueuedConnection)
 │        → QUEUE initializeEngineRenderer
 ├─ Component.onCompleted
 │    ├─ CSEditor.loadProject()
 │    │     └─ ProjectManager::CreateProject
 │    │         └─ ProjectModel ctor:
 │    │             ├─ create Scene (scene only; no sky/light yet)
 │    │             └─ if GetMainView()!=null → SetScene (else deferred)
 │    └─ appWindow.projectID = CSEditor.getProjectID()
 │        → binding → CSQuickRenderView::setProjectID
 │          (view null → stashed into m_pendingProjectID)
 └─ returns to event loop

[B] Event loop tick drains queued initializeEngineRenderer
 ├─ Acquire HWND = window()->winId()
 ├─ Acquire Qt HGLRC via QNativeInterface::QWGLContext::nativeContext
 ├─ GLRendererBuilder::SetNativeWindow(hwnd, qtHglrc)
 ├─ RenderModule::CreateRenderer(builder)
 │     → WGLWindowContext creates engine HGLRC, wglShareLists(qt, engine)
 │     → ViewGraph created, main View valid
 ├─ MakeContextCurrent on engine HGLRC → gladLoadGL → create color/depth
 │   textures sized to item (RGBA8Unorm + Depth32) → Build FBO eagerly
 │   (Qt RHI requires textures to be "specified" when sampled)
 ├─ ProjectManager::AttachAllToView(m_view)
 │     → binds Scene created in [A] to the main View
 ├─ Re-apply pending projectID (CameraManipulator built, no FitToScene yet)
 ├─ setActiveFocusOnTab(true) + forceActiveFocus(Qt::OtherFocusReason)
 └─ emit engineReady()                                            [C]

[C] QML onEngineReady → CSEditor.beginAssetLoading()
 ├─ project.initializeScene()
 │    ├─ PanoramicSky + HDR image (moonrise_puresky_4k.hdr)
 │    ├─ Directional Light (direction/color/intensity)
 │    └─ createSceneTreeModel()   ← SceneHierarchyView populated
 ├─ updateModels(project)         ← push hierarchy to QML
 ├─ project.startAssetLoad()      ← AssetManager::GetAssetSceneAsync
 └─ startAsyncLoadPolling(project)← QTimer 100ms poll

[D] Async future completes (background thread)
 └─ QTimer tick on main thread → project.pollAsyncLoad
     └─ future.get() → onAssetLoaded
         ├─ AssetImporter::Import(assetScene->GetRoot()) into Scene
         ├─ createSceneTreeModel() (rebuild hierarchy with imported nodes)
         └─ onAssetLoaded callback (QEditor):
             ├─ updateModels(project)
             └─ emit sceneAssetLoaded()                           [E]

[E] QML onSceneAssetLoaded → renderView.FitToScene(true)
 └─ CameraManipulator::FitTo(scene->GetAABB(recompute=true))
```

## 6. Runtime Frame Loop

After startup, every tick of `Engine::Run`:

```
main-thread tick
  ├─ RenderModule::Update
  │     ├─ RenderModuleContext ctx(this)
  │     ├─ SystemGraph::OnUpdate(ctx)   ECS (Transform/Camera/Light/MeshRender)
  │     ├─ Dispatch                     ECS command flush
  │     └─ renderer->Render(viewGraph)
  │           ├─ MakeContextCurrent(engine HGLRC)
  │           ├─ draw into engine FBO
  │           └─ DoneContextCurrent
  └─ QUIModule::Update
        ├─ Qt processEvents
        └─ if scene graph wants a repaint:
              QuickRenderView::updatePaintNode
                ├─ fetch GL id from engine color attachment
                ├─ QSGTexture = QNativeInterface::QSGOpenGLTexture
                               ::fromNative(glId, window, texSize,
                                            TextureHasAlphaChannel)
                ├─ node->setTexture; node->setRect(boundingRect())
                ├─ MirrorVertically  (engine is GL lower-left origin,
                                      QSG samples upper-left)
                └─ update()  (request another frame)
```

On `geometryChange`, `QuickRenderView::resizeRenderTarget` rebuilds the
engine textures + FBO at the new pixel size (device-pixel-ratio scaled).

## 7. Context Switching Rules

1. `wglMakeCurrent` is only called on the main thread.
2. Engine context is made current **only** while engine render work is in
   flight, then released with `DoneContextCurrent(NULL,NULL)` so Qt can
   re-acquire its context for QSG rendering.
3. All engine GL objects (textures, FBOs) are created while the engine
   context is current; `wglShareLists` was already called so Qt can sample
   them.
4. `NOMINMAX` is defined before `<windows.h>` in files that also include
   `<algorithm>` or use `std::max`.
5. `CSRender` links `opengl32 gdi32 user32` on Windows to resolve `wgl*` /
   `ChoosePixelFormat` / `GetDC`.

## 8. Focus & Input Handling

- Key events only reach `QuickRenderView::keyPressEvent` when the item has
  active focus.
- Three safe paths grant focus, none inside Qt's native-event flush:
  1. `mousePressEvent` calls `forceActiveFocus(Qt::MouseFocusReason)`.
  2. `initializeEngineRenderer` (runs on queued event-loop tick) calls
     `setActiveFocusOnTab(true)` + `forceActiveFocus(Qt::OtherFocusReason)`.
  3. QML sets `focus: true` on the `CSQuickRenderView` so it is the focus
     item of its scope.
- `setFocus(true)` / focus-policy calls are **not** placed in the
  constructor — those dispatch focus events synchronously during QML item
  instantiation and contribute to the recursive `notify` cascade.
- `activeFocusOnTab` is set from C++ only; exposing it as a QML property on
  the custom registered type reports an unavailable-property error
  (`CSEditor.View 255.255`).

## 9. Failure Modes Addressed

| Symptom | Root cause | Fix |
|---------|-----------|-----|
| `texD->texture && texD->specified` assert in `qrhigles2.cpp:5967` | Textures were created with default 1×1 size; FBO was never eagerly built, so Qt sampled a non-specified GL texture. | In `initializeEngineRenderer`, create textures at the item's pixel size and call `renderTarget.Build()` synchronously to `glTexImage2D` the storage. |
| Unbounded `notify()` recursion during startup | `m_qmlEngine.load()` ran inside `UIApplication`'s constructor; `sceneGraphInitialized` fired GL work inside Qt's synchronous signal frame; `setFocus` in ctor dispatched focus events during QML instantiation. All three feed `flushWindowSystemEvents → sendEvent → notify`. | (a) Defer QML load via `QTimer::singleShot(0,…)`; (b) use `Qt::QueuedConnection` for `sceneGraphInitialized`; (c) never call `setFocus`/focus-policy in the ctor — grant focus via mouse, queued init, and QML `focus: true`. |
| Null-dereference crash in `View::SetScene` at project load | `ProjectModel` constructor called `renderModule->GetMainView()->SetScene`, but the `View` did not exist yet because `initializeEngineRenderer` was queued. | Null-guard in ctor + `ProjectModel::GetScene()` + `ProjectManager::AttachAllToView`. Scene is attached inside `initializeEngineRenderer`. |
| `CameraManipulator` / `FitToScene` triggered on empty scene | `setProjectID` called `FitToScene(true)` before the asset was imported — the scene's AABB was degenerate. | Split into two events: `setProjectID` only builds the `CameraManipulator`; fit is deferred until `sceneAssetLoaded`, at which point the AABB reflects real geometry. |
| `keyPressEvent` silently ignored after focus was removed | Removing `setFocus(true)` to break the recursion also stripped focus acceptance. | Grant focus from three safe paths (see §8). |
| `activeFocusOnTab` rejected in QML | Qt reports property unavailable on our versioned custom type. | Declare `focus: true` in QML; call `setActiveFocusOnTab(true)` from C++. |
| Link errors `wglCreateContext / wglShareLists / …` | Windows opengl + gdi libs were not linked. | `if(WIN32) target_link_libraries(CSRender PRIVATE opengl32 gdi32 user32)`. |
| `std::max` fails to compile with `C2672` | `<windows.h>` defines `max`/`min` macros. | `#define NOMINMAX` before `<windows.h>`; include `<algorithm>`. |

## 10. QML Integration Surface

```qml
CSQuickRenderView {
    id: renderView
    projectID: appWindow.projectID
    anchors.fill: parent
    focus: true                          // focus item of the scope

    onEngineReady: CSEditor.beginAssetLoading()
}

Connections {
    target: CSEditor
    function onSceneAssetLoaded() { renderView.FitToScene(true) }
}
```

C++ surface exposed to QML:

- `Q_PROPERTY(QString projectID WRITE setProjectID)`
- `Q_INVOKABLE void setProjectID(const QString&)`
- `Q_INVOKABLE void FitToScene(bool recompute = false)`
- `Q_SIGNAL void engineReady()`
- QEditor: `Q_INVOKABLE void loadProject()`, `Q_INVOKABLE void beginAssetLoading()`, `Q_SIGNAL void sceneAssetLoaded()`

## 11. Acceptance Checks (matching the plan)

- Editor starts; 3D scene appears in the render panel; hierarchy and
  inspector behave as before.
- Resizing the window resizes the engine render target smoothly (no
  tearing / stale textures).
- Right-button rotate, middle pan/fly, wheel zoom/dolly, `W/S`, `F`,
  `Shift+F` all work.
- `RenderModule` contains no `<mutex>` / `<condition_variable>` symbols.
- No references to `QQuickRhiItem`, `QuickRenderer`, or `QBlitRenderPass`
  remain in the editor module.
- Engine context is created through `WGLWindowContext` and shares lists
  with Qt's RHI HGLRC (verified during `GraphicsGLImpl::Initialize`).
- `QuickRenderView::keyPressEvent` fires after either clicking the view,
  the window becoming active, or engine initialization completing.
- `ProjectModel::ProjectModel` survives a null `GetMainView()`; the scene
  is attached by `ProjectManager::AttachAllToView` after `engineReady`.
- `FitToScene` is only triggered after `sceneAssetLoaded`, when the AABB
  is valid.
