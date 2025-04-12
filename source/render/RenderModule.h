#pragma once
#include "core/IModule.h"

namespace CS
{

enum class EGraphicAPI
{
    OpenGL = 0,
    Max
};

class RenderModule : public IModule
{
public:
    RenderModule(EGraphicAPI type);
    ~RenderModule() override;

    virtual void Update() override;
};

} // namespace CS
