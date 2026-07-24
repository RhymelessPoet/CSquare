#pragma once
#include "CSAssert.h"
#include "LiteralString.h"
#include "Macros.h"
#include "TypeTraits.h"
#include <array>
#include <span>


namespace CS
{

template <typename EnumTag>
class Enum
{
public:
    CS_DELETE_NEW_OPERATORS

    static constexpr inline size_t NameLength = EnumTag::NameLength;
    static constexpr inline size_t MaxCount = EnumTag::MaxCount;

    using UnderlyingType = typename EnumTag::UnderlyingType;
    using NameType = LiteralString<NameLength>;

    template <LiteralString Str>
    static Enum Make()
    {
        return Enum(NameType(Str));
    }

    Enum() = default;

    bool IsUndefined() const { return m_value == 0; }

    operator std::string_view() const { return Names[m_value]; }
    explicit operator UnderlyingType() const { return m_value; }

    friend bool operator==(const Enum& lhs, const Enum& rhs) { return lhs.m_value == rhs.m_value; }

protected:
    Enum(const NameType& str)
    {
        for (UnderlyingType index = 0; index < Count; ++index) {
            if (Names[index] == str) {
                m_value = index;
                return;
            }
        }
        Assert(false, "Invalid enum value");
    }

    Enum(std::string_view name) : Enum(NameType(name)) {}

protected:
    static inline std::array<NameType, MaxCount> Names{std::string_view("undefined")};
    static inline UnderlyingType Count{1u};

private:
    UnderlyingType m_value{0};
};

template <typename EnumTag, LiteralString... Values>
struct EnumClass final : public Enum<EnumTag>
{
    using Base = Enum<EnumTag>;
    EnumClass()
    {
        Assert(sizeof...(Values) + Base::Count <= EnumTag::MaxCount, "Too many enum values");
        m_range.begin = Base::Count;
        auto checkUnique = [](auto& arr, auto& value) {
            for (typename Base::UnderlyingType i = 0; i < Base::Count; ++i) {
                if (arr[i] == value) {
                    Assert(false, "Duplicate enum value");
                }
            }
        };
        (checkUnique(Base::Names, Values), ...);
        auto pushBack = [](auto& arr, auto& value) {
            arr[Base::Count] = value;
            Base::Count += 1;
        };
        (pushBack(Base::Names, Values), ...);
        m_range.end = Base::Count;
    }

    bool Has(Base& e) const { return e.m_value >= m_range.begin && e.m_value < m_range.end; }

    std::span<const typename Base::NameType> GetSelfDefinedEnums() const
    {
        return std::span(Base::Names).subspan(m_range.begin, m_range.end - m_range.begin);
    }

    std::span<const typename Base::NameType> GetAllDefinedEnums() const
    {
        return std::span(Base::Names).subspan(0, Base::Count);
    }

    EnumClass(const EnumClass& other) = delete;
    EnumClass& operator=(const EnumClass& other) = delete;
    CS_DELETE_NEW_OPERATORS

private:
    struct
    {
        size_t begin{0};
        size_t end{0};
    } m_range;
};

template <typename EnumType>
    requires(type_traits::is_template_instance_v<EnumType, Enum>)
struct underlying_type
{
    using type = typename EnumType::UnderlyingType;
};

template <typename EnumTag>
using underlying_type_t = typename underlying_type<Enum<EnumTag>>::type;

} // namespace CS

namespace std
{

template <typename EnumTag>
struct hash<CS::Enum<EnumTag>>
{
    size_t operator()(const CS::Enum<EnumTag>& e) const
    {
        return std::hash<typename CS::Enum<EnumTag>::UnderlyingType>()(
            static_cast<typename CS::Enum<EnumTag>::UnderlyingType>(e));
    }
};

} // namespace std
