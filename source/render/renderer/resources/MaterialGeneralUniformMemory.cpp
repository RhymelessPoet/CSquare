#include "MaterialGeneralUniformMemory.h"
#include "graphics/GraphicsAPI.h"

namespace CS
{
MaterialGeneralUniformMemory::MaterialGeneralUniformMemory(std::shared_ptr<GraphicsAPI> graphicsAPI,
                                                           size_t preferredSize)
    : m_memory(preferredSize)
{
    m_uniformBuffer = graphicsAPI->CreateUniformBuffer(preferredSize);
}

size_t MaterialGeneralUniformMemory::Allocate(size_t size)
{
    if (m_offset + size > m_memory.size()) {
        m_memory.resize(m_memory.size() + std::max(size, static_cast<size_t>(41943040)));
    }

    auto preOffset = m_offset;

    m_offset += size;

    auto mod = m_offset % 256;

    if (mod != 0) {
        m_offset += (256 - mod);
    }

    return preOffset;
}

std::span<std::byte> MaterialGeneralUniformMemory::AllocateUniform(std::string_view name, size_t offset, size_t size)
{
    m_uniforms.emplace(name, MemoryView{offset, size});
    m_dirty = true;
    return std::span<std::byte>(m_memory.data() + offset, size);
}

std::span<std::byte> MaterialGeneralUniformMemory::GetUniformMemory(std::string_view name)
{
    auto itr = m_uniforms.find(std::string(name));
    if (itr != m_uniforms.end()) {
        const auto& [offset, size] = itr->second;
        return std::span<std::byte>(m_memory.data() + offset, size);
    }
    return std::span<std::byte>();
}

bool MaterialGeneralUniformMemory::SetUniformMemory(std::string_view name, std::vector<std::byte> memory)
{
    auto itr = m_uniforms.find(std::string(name));
    if (itr != m_uniforms.end()) {
        if (memory.size() <= itr->second.size) {
            const auto& [offset, size] = itr->second;
            auto data = std::span<std::byte>(m_memory.data() + offset, size);
            std::copy(memory.begin(), memory.end(), data.begin());
            m_dirty = true;
            return true;
        }
    }
    return false;
}

void MaterialGeneralUniformMemory::UpdateUniformBuffer()
{
    if (!m_uniformBuffer.IsBuild()) {
        m_uniformBuffer.Build();
    }

    if (m_dirty) {
        m_uniformBuffer.UpdateData(m_memory.data(), m_offset);
        m_dirty = false;
    }
}

} // namespace CS
