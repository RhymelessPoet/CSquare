#include "IRenderSample.h"
#include "scene/CameraSystem.h"
#include "scene/MeshRenderSystem.h"
#include "scene/MeshRenderer.h"
#include "scene/SceneObjectComposer.h"
#include "scene/Transform.h"
#include "scene/TransformSystem.h"

#include <random>

namespace CS
{

Vector3f CS::IRenderSample::RandomPosition(const Vector3f& minRange, const Vector3f& maxRange)
{
    std::random_device randomDevice;
    std::mt19937 generate(randomDevice());

    Vector3f position;

    for (uint32_t index = 0u; index < 3; ++index) {
        std::uniform_real_distribution<float> distribution(minRange[index], maxRange[index]);
        position[index] = distribution(generate);
    }

    return position;
}

std::vector<Vector3f> IRenderSample::RandomPositions(uint32_t count, const Vector3f& minRange, const Vector3f& maxRange)
{
    std::vector<Vector3f> positions;

    std::random_device randomDevice;
    std::mt19937 generate(randomDevice());

    for (uint32_t i = 0u; i < count; ++i) {
        Vector3f position;

        for (uint32_t j = 0u; j < 3u; ++j) {
            std::uniform_real_distribution<float> distribution(minRange[j], maxRange[j]);
            position[j] = distribution(generate);
        }

        positions.emplace_back(position);
    }

    return positions;
}

IRenderSample::IRenderSample(std::shared_ptr<SceneObjectComposer> composer) : m_composer(composer) {}

bool IRenderSample::createMeshRenderer(std::shared_ptr<SceneObject> object)
{
    return m_composer->AddComponent<MeshRenderer>(object);
}

bool IRenderSample::createTransform(std::shared_ptr<SceneObject> object)
{
    return m_composer->AddComponent<Transform>(object);
}

} // namespace CS