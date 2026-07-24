#pragma once
#include "CSAssert.h"
#include "LiteralString.h"
#include "Macros.h"
#include "TypeTraits.h"
#include <array>
#include <bitset>
#include <span>


namespace CS
{

template <typename EnumTag>
class EnumFlags;

template <typename EnumTag>
class FlagEnum
{
public:
    CS_DELETE_NEW_OPERATORS

    static constexpr inline size_t NameLength = EnumTag::NameLength;
    static constexpr inline size_t MaxCount = EnumTag::MaxCount;

    using UnderlyingType = typename EnumTag::UnderlyingType;
    using NameType = LiteralString<NameLength>;
    using Bitset = std::bitset<MaxCount>;

    template <LiteralString Str>
    static FlagEnum Make()
    {
        return FlagEnum(NameType(Str));
    }

    FlagEnum() = default;

    bool IsUndefined() const { return m_value == 0; }

    operator std::string_view() const
    {
        Assert(m_value < Count, "FlagEnum value out of registered range");
        return Names[m_value];
    }

    explicit operator UnderlyingType() const { return m_value; }

    friend bool operator==(const FlagEnum& lhs, const FlagEnum& rhs) { return lhs.m_value == rhs.m_value; }
    friend bool operator!=(const FlagEnum& lhs, const FlagEnum& rhs) { return lhs.m_value != rhs.m_value; }

    EnumFlags<EnumTag> operator|(const FlagEnum& rhs) const;
    EnumFlags<EnumTag> operator|(const EnumFlags<EnumTag>& rhs) const;

protected:
    FlagEnum(const NameType& str)
    {
        for (UnderlyingType index = 0; index < Count; ++index) {
            if (Names[index] == str) {
                m_value = index;
                return;
            }
        }
        Assert(false, "Invalid flag enum value");
    }

    FlagEnum(std::string_view name) : FlagEnum(NameType(name)) {}

protected:
    // Names[0] = "undefined", Names[1] = first flag, Names[2] = second flag, ...
    static inline std::array<NameType, MaxCount> Names{std::string_view("undefined")};
    static inline UnderlyingType Count{1u};
    // Bitset where bit N is set iff the flag at Names[N+1] is registered.
    // Used by EnumFlags for operator~() and All().
    static inline Bitset AllFlagsMask{};

private:
    // Sequential index: 0 = undefined, 1 = first flag, 2 = second flag, ...
    UnderlyingType m_value{0};

    friend class EnumFlags<EnumTag>;
};

template <typename EnumTag>
class EnumFlags
{
public:
    CS_DELETE_NEW_OPERATORS

    using UnderlyingType = typename FlagEnum<EnumTag>::UnderlyingType;
    using Bitset = typename FlagEnum<EnumTag>::Bitset;

    EnumFlags() = default;

    EnumFlags(const FlagEnum<EnumTag>& e)
    {
        auto index = static_cast<UnderlyingType>(e);
        if (index > 0) {
            m_bits.set(index - 1);
        }
    }

    EnumFlags operator|(const EnumFlags& rhs) const { return fromBitset(m_bits | rhs.m_bits); }
    EnumFlags operator|(const FlagEnum<EnumTag>& rhs) const
    {
        EnumFlags result(*this);
        result.Set(rhs);
        return result;
    }
    EnumFlags operator&(const EnumFlags& rhs) const { return fromBitset(m_bits & rhs.m_bits); }
    EnumFlags operator&(const FlagEnum<EnumTag>& rhs) const { return fromBitset(m_bits & singleBit(rhs)); }
    EnumFlags operator^(const EnumFlags& rhs) const { return fromBitset(m_bits ^ rhs.m_bits); }
    EnumFlags operator~() const { return fromBitset(~m_bits & FlagEnum<EnumTag>::AllFlagsMask); }

    EnumFlags& operator|=(const EnumFlags& rhs)
    {
        m_bits |= rhs.m_bits;
        return *this;
    }
    EnumFlags& operator|=(const FlagEnum<EnumTag>& rhs)
    {
        Set(rhs);
        return *this;
    }
    EnumFlags& operator&=(const EnumFlags& rhs)
    {
        m_bits &= rhs.m_bits;
        return *this;
    }
    EnumFlags& operator^=(const EnumFlags& rhs)
    {
        m_bits ^= rhs.m_bits;
        return *this;
    }

    bool Test(const FlagEnum<EnumTag>& flag) const
    {
        auto index = static_cast<UnderlyingType>(flag);
        if (index == 0)
            return false;
        return m_bits.test(index - 1);
    }
    bool Test(const EnumFlags& flags) const { return (m_bits & flags.m_bits).any(); }
    bool Any() const { return m_bits.any(); }
    bool None() const { return m_bits.none(); }
    bool All() const { return (m_bits & FlagEnum<EnumTag>::AllFlagsMask) == FlagEnum<EnumTag>::AllFlagsMask; }

    void Set(const FlagEnum<EnumTag>& flag)
    {
        auto index = static_cast<UnderlyingType>(flag);
        if (index > 0) {
            m_bits.set(index - 1);
        }
    }
    void Reset(const FlagEnum<EnumTag>& flag)
    {
        auto index = static_cast<UnderlyingType>(flag);
        if (index > 0) {
            m_bits.reset(index - 1);
        }
    }
    void Reset() { m_bits.reset(); }

    explicit operator UnderlyingType() const { return static_cast<UnderlyingType>(m_bits.to_ullong()); }
    explicit operator bool() const { return m_bits.any(); }

    unsigned long long ToUllong() const { return m_bits.to_ullong(); }

    bool operator==(const EnumFlags& rhs) const { return m_bits == rhs.m_bits; }
    bool operator!=(const EnumFlags& rhs) const { return m_bits != rhs.m_bits; }

private:
    explicit EnumFlags(const Bitset& bits) : m_bits(bits) {}
    static EnumFlags fromBitset(const Bitset& bits) { return EnumFlags(bits); }

    static Bitset singleBit(const FlagEnum<EnumTag>& e)
    {
        Bitset bits;
        auto index = static_cast<UnderlyingType>(e);
        if (index > 0) {
            bits.set(index - 1);
        }
        return bits;
    }

    Bitset m_bits;

    friend class FlagEnum<EnumTag>;
};

// --- Out-of-line FlagEnum member definitions (require EnumFlags to be complete) ---

template <typename EnumTag>
inline EnumFlags<EnumTag> FlagEnum<EnumTag>::operator|(const FlagEnum& rhs) const
{
    EnumFlags<EnumTag> result(*this);
    result.Set(rhs);
    return result;
}

template <typename EnumTag>
inline EnumFlags<EnumTag> FlagEnum<EnumTag>::operator|(const EnumFlags<EnumTag>& rhs) const
{
    EnumFlags<EnumTag> result(rhs);
    result.Set(*this);
    return result;
}

// --- FlagEnumClass ---

template <typename EnumTag, LiteralString... Values>
struct FlagEnumClass final : public FlagEnum<EnumTag>
{
    using Base = FlagEnum<EnumTag>;

    FlagEnumClass()
    {
        Assert(sizeof...(Values) + Base::Count <= EnumTag::MaxCount, "Too many flag enum values");

        m_range.begin = Base::Count;

        auto checkUnique = [](auto& arr, auto& value) {
            for (typename Base::UnderlyingType i = 0; i < Base::Count; ++i) {
                if (arr[i] == value) {
                    Assert(false, "Duplicate flag enum value");
                }
            }
        };
        (checkUnique(Base::Names, Values), ...);

        auto pushBack = [](auto& arr, auto& value) {
            arr[Base::Count] = value;
            Base::AllFlagsMask.set(Base::Count - 1);
            Base::Count += 1;
        };
        (pushBack(Base::Names, Values), ...);

        m_range.end = Base::Count;
    }

    bool Has(Base& e) const
    {
        auto val = static_cast<typename Base::UnderlyingType>(e);
        return val >= m_range.begin && val < m_range.end;
    }

    std::span<const typename Base::NameType> GetSelfDefinedEnums() const
    {
        return std::span(Base::Names).subspan(m_range.begin, m_range.end - m_range.begin);
    }

    std::span<const typename Base::NameType> GetAllDefinedEnums() const
    {
        return std::span(Base::Names).subspan(0, Base::Count);
    }

    FlagEnumClass(const FlagEnumClass&) = delete;
    FlagEnumClass& operator=(const FlagEnumClass&) = delete;
    CS_DELETE_NEW_OPERATORS

private:
    struct
    {
        size_t begin{0};
        size_t end{0};
    } m_range;
};

// --- Type traits ---

template <typename EnumType>
    requires(type_traits::is_template_instance_v<EnumType, FlagEnum>)
struct flag_underlying_type
{
    using type = typename EnumType::UnderlyingType;
};

template <typename EnumTag>
using flag_underlying_type_t = typename flag_underlying_type<FlagEnum<EnumTag>>::type;

} // namespace CS

namespace std
{

template <typename EnumTag>
struct hash<CS::EnumFlags<EnumTag>>
{
    size_t operator()(const CS::EnumFlags<EnumTag>& flags) const
    {
        return std::hash<unsigned long long>()(flags.ToUllong());
    }
};

} // namespace std
