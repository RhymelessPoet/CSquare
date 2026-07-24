#pragma once
#include <queue>
#include <type_traits>

namespace CS
{

template <typename IDType = uint32_t,
          uint32_t IDBits = sizeof(IDType) * 6,
          uint32_t GenerationBits = sizeof(IDType) * 2>
    requires(std::is_unsigned_v<IDType> && IDBits + GenerationBits <= sizeof(IDType) * 8 && IDBits > 0 &&
             GenerationBits > 0)
class GenerationID final
{
public:
    class Generator
    {
    public:
        GenerationID Allocate()
        {
            if (!m_freeIDs.empty()) {
                GenerationID id = m_freeIDs.front();
                m_freeIDs.pop();
                return id;
            }
            return GenerationID(m_currentID++);
        }

        void Free(const GenerationID& id)
        {
            if (id.m_generation + 1 == (1u << GenerationBits)) {
                // Cannot recycle ID anymore
                return;
            }
            m_freeIDs.push(GenerationID(id.m_id, id.m_generation + 1));
        }

    private:
        IDType m_currentID{0};
        std::queue<GenerationID> m_freeIDs;
    };

    GenerationID(/* args */) : m_id(0), m_generation(0) {}
    GenerationID(IDType id, IDType generation = 1) : m_id(id), m_generation(generation) {}

    bool IsValid() const { return m_generation != 0; }
    bool operator==(const GenerationID& other) const
    {
        return m_id == other.m_id && m_generation == other.m_generation;
    }

    bool operator!=(const GenerationID& other) const { return !(*this == other); }

    bool operator<(const GenerationID& other) const
    {
        if (m_id == other.m_id) {
            return m_generation < other.m_generation;
        }
        return m_id < other.m_id;
    }

    explicit operator IDType() const { return m_id << GenerationBits | m_generation; }

    IDType Hash() const { return static_cast<IDType>(*this); }

private:
    IDType m_id : IDBits;
    IDType m_generation : GenerationBits{0u};
};

} // namespace CS

template <typename T>
constexpr bool is_template_instance_GenerationID_v = false;

template <typename IDType, uint32_t IDBits, uint32_t GenerationBits>
constexpr bool is_template_instance_GenerationID_v<CS::GenerationID<IDType, IDBits, GenerationBits>> = true;

template <typename T>
concept GenerationIDType = is_template_instance_GenerationID_v<T>;

namespace std
{
template <typename IDType, uint32_t IDBits, uint32_t GenerationBits>
struct hash<CS::GenerationID<IDType, IDBits, GenerationBits>>
{
    std::size_t operator()(const CS::GenerationID<IDType, IDBits, GenerationBits>& id) const
    {
        return std::hash<IDType>{}((IDType)id);
    }
};
} // namespace std