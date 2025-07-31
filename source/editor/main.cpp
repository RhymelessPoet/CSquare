#include "Engine.h"
#include "QUIModule.h"
#include "RenderModule.h"
#include "graphics/GraphicsDefine.h"

int main(int argc, char* argv[])
{
    auto& engine = CS::Engine::Instance();

    engine.AddModule<CS::RenderModule>();
    engine.AddModule<CS::QUIModule>(argc, argv);

    engine.Run();

    return 0;
}
