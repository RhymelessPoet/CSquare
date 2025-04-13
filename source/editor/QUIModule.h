#pragma once
#include "core/IModule.h"
namespace CS
{
class QUIModuleImpl;
class QUIModule : public IModule
{
public:
    QUIModule(int argc, char* argv[]);
    ~QUIModule() = default;

    virtual void Update() override;
    virtual void SetEngineController(std::shared_ptr<EngineController> controller);

private:
    QUIModuleImpl& impl();

private:
    std::unique_ptr<QUIModuleImpl> m_impl;
};

} // namespace CS
