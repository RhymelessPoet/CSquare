#pragma once
#include "base/Singleton.h"
#include <memory>

namespace CS
{
class Material;

class BuiltInMaterials : public Singleton<BuiltInMaterials>
{
public:
    std::shared_ptr<Material> GetPBRMaterial();

private:
    std::shared_ptr<Material> createPBRMaterial();

private:
    std::shared_ptr<Material> m_pbrMaterial;
};

} // namespace CS
