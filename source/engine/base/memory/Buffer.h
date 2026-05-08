#pragma once
#include <span>
#include <vector>

namespace CS
{

class Buffer
{
public:
    Buffer() = default;
    Buffer(size_t capacity) { m_data.reserve(capacity); };
    Buffer(std::vector<std::byte> data) : m_data(std::move(data)) {}

    template <typename T>
        requires(std::is_trivially_copyable_v<T>)
    Buffer(const std::vector<T>& data)
    {
        PushBack(data);
    }

    template <typename T>
        requires(std::is_trivially_copyable_v<T>)
    Buffer(const T* data, size_t count) : m_data(sizeof(T) * count)
    {
        PushBack(data, count);
    }

    template <typename T>
        requires(std::is_trivially_copyable_v<T>)
    inline std::span<const T> GetData() const
    {
        return std::span<const T>(reinterpret_cast<const T*>(m_data.data()), m_data.size() / sizeof(T));
    }

    inline std::span<const std::byte> GetByteData() const
    {
        return std::span<const std::byte>(m_data.data(), m_data.size());
    }

    size_t GetByteSize() const { return m_data.size(); }
    size_t GetByteCapacity() const { return m_data.capacity(); }

    void Reserve(size_t capacity) { m_data.reserve(capacity); }

    template <typename T>
        requires(std::is_trivially_copyable_v<T>)
    inline void PushBack(const T* data, size_t count)
    {
        size_t byteSize = sizeof(T) * count;
        pushBackBytes(reinterpret_cast<const std::byte*>(data), byteSize);
    }

    template <typename T>
        requires(std::is_trivially_copyable_v<T>)
    inline void PushBack(const std::vector<T>& data)
    {
        size_t byteSize = sizeof(T) * data.size();
        pushBackBytes(reinterpret_cast<const std::byte*>(data.data()), byteSize);
    }

private:
    inline void pushBackBytes(const std::byte* data, size_t size)
    {
        size_t currentSize = m_data.size();
        m_data.resize(currentSize + size);
        std::memcpy(m_data.data() + currentSize, data, size);
    }

private:
    std::vector<std::byte> m_data;
};

} // namespace CS
