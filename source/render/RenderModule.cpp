#include "RenderModule.h"
#include "graphics/opengl/glad/include/glad/glad.h"

namespace CS
{
RenderModule::RenderModule(EGraphicAPI type)
{
    gladLoadGL();
}

RenderModule::~RenderModule() {}

void RenderModule::Update()
{
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

} // namespace CS
