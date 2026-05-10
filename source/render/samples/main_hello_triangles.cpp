#include "Engine.h"
#include "RenderModule.h"
#include "SHelloTriangles.h"
#include "SampleModule.h"
#include "base/Logger.h"

int main(int /*argc*/, char* /*argv*/[])
{
    CS::Logger::Initialize();

    auto& engine = CS::Engine::Instance();
    engine.AddModule<CS::RenderModule>();
    engine.AddModule<CS::SampleModule>(
        std::string("HelloTriangles"), CS::Size2u{1280u, 720u},
        [](std::shared_ptr<CS::SceneObjectComposer> composer) -> std::unique_ptr<CS::IRenderSample> {
            return std::make_unique<CS::SHelloTriangles>(std::move(composer));
        });
    engine.Run();

    CS::Logger::Shutdown();
    return 0;
}
