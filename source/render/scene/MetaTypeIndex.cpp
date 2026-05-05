#include "MetaTypeIndex.h"

#include <mutex>
#include <typeindex>
#include <unordered_map>

namespace CS
{

namespace
{
struct Registry
{
    std::unordered_map<std::type_index, Ubpa::Type> map;
    std::mutex mutex;
};

Registry& registry()
{
    // Meyers' singleton: safe to call from any static initializer.
    static Registry r;
    return r;
}
} // namespace

void MetaTypeIndex::Register(const std::type_info& ti, Ubpa::Type type)
{
    auto& r = registry();
    std::lock_guard<std::mutex> lock(r.mutex);
    r.map.insert_or_assign(std::type_index(ti), type);
}

Ubpa::Type MetaTypeIndex::Resolve(const std::type_info& ti)
{
    auto& r = registry();
    std::lock_guard<std::mutex> lock(r.mutex);
    auto it = r.map.find(std::type_index(ti));
    if (it == r.map.end()) {
        return {};
    }
    return it->second;
}

} // namespace CS
