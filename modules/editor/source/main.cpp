#include "LoggerConfig.h"
#include "QUIModule.h"
#include "RenderModule.h"
#include "engine/Engine.h"
#include "graphics/GraphicsDefine.h"


int main(int argc, char* argv[])
{
    CSEditor::LoggerConfig::Load("editor_config.json").Apply();

    auto& engine = CS::Engine::Instance();

    engine.AddModule<CS::RenderModule>();
    engine.AddModule<CS::QUIModule>(argc, argv);

    engine.Run();

    CS::Logger::Shutdown();

    return 0;
}
