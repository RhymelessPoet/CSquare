#include "Engine.h"
#include "QUIModule.h"
#include "RenderModule.h"

int main(int argc, char* argv[])
{
    CS::Engine engine;
    engine.addModule<CS::QUIModule>(argc, argv);
    engine.addModule<CS::RenderModule>(CS::EGraphicAPI::OpenGL);

    engine.Run();

    return 0;
}
