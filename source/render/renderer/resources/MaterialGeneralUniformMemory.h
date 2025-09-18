#pragma once
#include "base/Macros.h"
#include "graphics/UniformBuffer.h"
#include <map>
#include <span>
#include <string>
#include <vector>

namespace CS
{

class MaterialGeneralUniformMemory
{
public:
    struct MemoryView
    {
        size_t offset{0u};
        size_t size{0u};
    };
    explicit MaterialGeneralUniformMemory(std::shared_ptr<GraphicsAPI> graphicsAPI, size_t preferredSize = 4194304);

    CS_DELETE_COPY_MOVE(MaterialGeneralUniformMemory);

    size_t Allocate(size_t size);
    std::span<std::byte> AllocateUniform(std::string_view name, size_t offset, size_t size);

    std::span<std::byte> GetUniformMemory(std::string_view name);
    bool SetUniformMemory(std::string_view name, std::vector<std::byte> memory);

    void UpdateUniformBuffer();
    UniformBuffer GetUniformBuffer() { return m_uniformBuffer; }

private:
    size_t m_offset{0u};
    bool m_dirty{false};
    std::map<std::string, MemoryView> m_uniforms;
    std::vector<std::byte> m_memory;
    UniformBuffer m_uniformBuffer;
};

} // namespace CS
