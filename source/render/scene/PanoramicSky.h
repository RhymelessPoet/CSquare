#pragma once
#include <memory>

namespace CS
{

class Scene;
class SceneObject;
class Image;
class MaterialInstance;

class PanoramicSky
{
public:
    PanoramicSky(std::shared_ptr<Scene> scene);
    ~PanoramicSky();

    void SetImage(std::shared_ptr<Image> image);

private:
    std::shared_ptr<Scene> m_scene;
    std::shared_ptr<SceneObject> m_sceneObject;
    std::shared_ptr<MaterialInstance> m_material;
};

} // namespace CS
