#pragma once
#include <memory>
#include <vector>
namespace CS
{
class ISystem;
class InvalidSystem;
class IRenderSystem;
class RenderContext;
class SceneObject;
class Camera;
class Scene : public std::enable_shared_from_this<Scene>
{
public:
    Scene(/* args */);
    ~Scene();

    void OnUpdate();
    void OnRender(RenderContext& context);

    std::shared_ptr<SceneObject> GetRoot() { return m_root; }

    std::shared_ptr<Camera> CreateCamera();

    template <typename SystemType, typename... Args>
        requires std::derived_from<SystemType, ISystem>
    bool AddSystem(Args... args)
    {
        if (GetSystem<SystemType>().IsValid()) {
            return false; // System already exists
        }
        auto system = std::make_unique<SystemType>(std::forward<Args>(args)...);
        if constexpr (std::derived_from<SystemType, IRenderSystem>) {
            m_renderSystems.push_back(system.get());
        }
        m_systems.push_back(std::move(system));
        return true;
    }

    template <typename SystemType, typename... Args>
        requires std::derived_from<SystemType, ISystem>
    ISystem& GetSystem()
    {
        for (auto& system : m_systems) {
            if (auto castedSystem = dynamic_cast<SystemType*>(system.get())) {
                return *castedSystem;
            }
        }
        return InvalidSystem::Instance();
    }

    std::shared_ptr<SceneObject> CreateSceneObject(std::shared_ptr<SceneObject> parent = nullptr);

private:
    std::vector<std::unique_ptr<ISystem>> m_systems;
    std::vector<IRenderSystem*> m_renderSystems;
    std::shared_ptr<SceneObject> m_root;
};

} // namespace CS
