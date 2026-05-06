# ExtensibleEnum & ExtensibleFlagEnum 设计与使用

## 目录

- [1. 概述](#1-概述)
- [2. 设计理念](#2-设计理念)
- [3. ExtensibleEnum — 可扩展单值枚举](#3-extensibleenum--可扩展单值枚举)
  - [3.1 源代码](#31-源代码)
  - [3.2 值模型](#32-值模型)
  - [3.3 EnumTag 约定](#33-enumtag-约定)
  - [3.4 使用示例](#34-使用示例)
  - [3.5 API 参考](#35-api-参考)
- [4. ExtensibleFlagEnum — 可扩展标志枚举](#4-extensibleflagenum--可扩展标志枚举)
  - [4.1 源代码](#41-源代码)
  - [4.2 值模型与位映射](#42-值模型与位映射)
  - [4.3 EnumTag 约定](#43-enumtag-约定)
  - [4.4 使用示例](#44-使用示例)
  - [4.5 API 参考 — FlagEnum](#45-api-参考--flagenum)
  - [4.6 API 参考 — EnumFlags](#46-api-参考--enumflags)
- [5. 两者对比](#5-两者对比)
- [6. 扩展与跨模块注册](#6-扩展与跨模块注册)
- [7. 线程安全与初始化顺序](#7-线程安全与初始化顺序)

---

## 1. 概述

| 组件 | 头文件 | 用途 |
|------|--------|------|
| **ExtensibleEnum** | `base/ExtensibleEnum.h` | 单值枚举 — 每个实例持有并代表一个离散值 |
| **ExtensibleFlagEnum** | `base/ExtensibleFlagEnum.h` | 标志枚举 — 值可组合，支持 “\|” 位或运算，自动产生 `EnumFlags` 位集类型 |

两者共享相同的设计范式：通过 **EnumTag** 结构体声明元信息，通过 **Class** 模板在命名空间/模块作用域注册值，运行时按名称查找。区别在于 FlagEnum 额外产生 `EnumFlags` 位集类型以支持标志组合。

---

## 2. 设计理念

1. **声明式注册**：枚举值以编译期字符串字面量注册，无需手写数值。
2. **跨模块扩展**：同一 EnumTag 可被多个 `EnumClass` / `FlagEnumClass` 分别注册，值自动递增。
3. **运行时名称查找**：每个值可通过 `operator std::string_view()` 获取其名称。
4. **类型安全**：不同 EnumTag 产生不同实例化类型，编译期隔离。
5. **值语义**：`Enum` / `FlagEnum` / `EnumFlags` 均为值类型，默认可复制。

---

## 3. ExtensibleEnum — 可扩展单值枚举

### 3.1 源代码

```cpp
// source/engine/base/ExtensibleEnum.h
#pragma once
#include "Assert.h"        // Assert(condition, message) — 条件为 false 时抛出 std::runtime_error
#include "LiteralString.h" // LiteralString<N> — 固定长度编译期字符串，支持构造自 const char[N]
                           //   和 std::string_view，可隐式转为 std::string_view
#include "TypeTraits.h"    // type_traits::is_template_instance_v<T, Template> — 判断 T 是否为
                           //   Template 的实例化，用于 underlying_type 的 requires 约束
#include "base/Macros.h"   // CS_DELETE_NEW_OPERATORS — 删除 operator new / new[]
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
```

### 3.2 值模型

`Enum::m_value` 为 **顺序索引**：

| 索引 | 含义 | 名称 |
|------|------|------|
| 0 | undefined（默认值） | `"undefined"` |
| 1 | 第一个注册值 | 用户定义 |
| 2 | 第二个注册值 | 用户定义 |
| ... | ... | ... |

### 3.3 EnumTag 约定

用户需定义一个结构体，包含以下三个静态成员：

```cpp
struct MyEnumTag
{
    using UnderlyingType = uint8_t;         // 存储 m_value 的整数类型
    static constexpr size_t NameLength = 16; // 每个名称的最大字符数（含 '\0'）
    static constexpr size_t MaxCount = 32;   // 最大条目数（含 "undefined"）
};
```

- `UnderlyingType` 只需能容纳 `0` ~ `MaxCount - 1`，`uint8_t` 可支持 256 个条目。
- `MaxCount` 决定了 `Names` 数组的大小。

### 3.4 使用示例

```cpp
#include "base/ExtensibleEnum.h"

// 1. 定义 Tag
struct ViewType
{
    using UnderlyingType = uint8_t;
    static constexpr size_t NameLength{16u};
    static constexpr size_t MaxCount = 32u;
};

// 2. 注册值（通常在 .h 文件中，命名空间作用域）
const inline EnumClass<ViewType, "3D_Main", "Shadow_Map"> PreDefineViewTypes;

// 3. 使用类型别名
using EViewType = Enum<ViewType>;

// 4. 创建与使用
auto mainView = EViewType::Make<"3D_Main">();
std::string_view name = mainView;           // "3D_Main"
bool undef = mainView.IsUndefined();        // false
auto val = static_cast<uint8_t>(mainView);  // 1
```

### 3.5 API 参考

#### `Enum<EnumTag>`

| 成员 | 可见性 | 说明 |
|------|--------|------|
| `Make<Str>()` | public static | 按名称创建枚举值 |
| `IsUndefined()` | public | 是否为 undefined（索引 0） |
| `operator std::string_view()` | public | 获取当前值名称 |
| `explicit operator UnderlyingType()` | public | 获取顺序索引 |
| `operator==` | public friend | 相等比较 |

#### `EnumClass<EnumTag, Values...>`

| 成员 | 可见性 | 说明 |
|------|--------|------|
| 构造函数 | public | 注册值并检测重复 |
| `Has(e)` | public | 判断枚举值是否属于本 Class 注册的范围 |
| `GetSelfDefinedEnums()` | public | 返回本 Class 注册的名称 span |
| `GetAllDefinedEnums()` | public | 返回所有已注册的名称 span |

#### 类型特征

```cpp
underlying_type_t<Enum<EnumTag>>  // → EnumTag::UnderlyingType
```

#### `std::hash` 特化

`Enum<EnumTag>` 已提供 `std::hash` 特化，可直接用于无序容器。

---

## 4. ExtensibleFlagEnum — 可扩展标志枚举

### 4.1 源代码

```cpp
// source/engine/base/ExtensibleFlagEnum.h
#pragma once
#include "Assert.h"        // Assert(condition, message) — 条件为 false 时抛出 std::runtime_error
#include "LiteralString.h" // LiteralString<N> — 固定长度编译期字符串，用作 EnumClass / FlagEnumClass
                           //   的模板参数包 Values 的类型，也用作 Names 数组元素类型
#include "TypeTraits.h"    // type_traits::is_template_instance_v<T, Template> — 用于
                           //   flag_underlying_type 的 requires 约束
#include "base/Macros.h"   // CS_DELETE_NEW_OPERATORS — 删除 operator new / new[]
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
```

### 4.2 值模型与位映射

#### FlagEnum — 顺序索引

`FlagEnum::m_value` 与 `Enum` 相同，为 **顺序索引**：

| 索引 | 含义 | 名称 |
|------|------|------|
| 0 | undefined | `"undefined"` |
| 1 | 第一个标志 | 用户定义 |
| 2 | 第二个标志 | 用户定义 |
| ... | ... | ... |

> **关键设计**：FlagEnum 不存储 2 的幂值。它使用与 Enum 相同的顺序索引，到 bitset 位位置的映射仅在 EnumFlags 边界发生。

#### EnumFlags — bitset 位映射

`EnumFlags` 内部使用 `std::bitset<MaxCount>` 存储，映射关系为：

```
FlagEnum index N  →  bitset bit position (N - 1)
     0 (undefined)  →  无位
     1              →  bit 0
     2              →  bit 1
     3              →  bit 2
     ...            →  ...
```

因此 `MaxCount - 1` 是可注册标志数的上限，`bitset<MaxCount>` 提供足够的位空间。

#### 三者关系

```
UnderlyingType  — FlagEnum::m_value / Count 的存储类型，只需容纳 0..MaxCount-1
                  uint8_t 可支持 ≤256 个条目
MaxCount        — Names 数组总大小（含 "undefined"）
                  可注册标志数上限 = MaxCount - 1
bitset<MaxCount>— EnumFlags 内部存储
                  有效位 = MaxCount - 1 个（bit 0 ~ bit MaxCount-2）
```

### 4.3 EnumTag 约定

与 ExtensibleEnum 完全相同：

```cpp
struct MyFlagTag
{
    using UnderlyingType = uint8_t;      // 索引类型，只需容纳 0..MaxCount-1
    static constexpr size_t NameLength = 16;
    static constexpr size_t MaxCount = 32;
};
```

### 4.4 使用示例

```cpp
#include "base/ExtensibleFlagEnum.h"

// 1. 定义 Tag
struct ComponentState
{
    using UnderlyingType = uint8_t;
    static constexpr size_t NameLength{16u};
    static constexpr size_t MaxCount = 32u;
};

// 2. 注册值
const inline FlagEnumClass<ComponentState, "New", "Active", "Destroyed"> BaseComponentStates;

// 3. 使用类型别名
using EComponentState = FlagEnum<ComponentState>;
using EComponentStateFlags = EnumFlags<ComponentState>;

// 4. 创建标志
auto stateNew     = EComponentState::Make<"New">();      // index = 1 → bit 0
auto stateActive  = EComponentState::Make<"Active">();   // index = 2 → bit 1

// 5. 组合标志（operator|）
auto combined = stateNew | stateActive;                  // → EnumFlags, bit 0 & bit 1
bool hasNew = combined.Test(stateNew);                    // true
bool hasAll = combined.All();                             // false（缺少 Destroyed）

// 6. EnumFlags 修改
EComponentStateFlags flags;
flags.Set(stateNew);                                     // 设置 bit 0
flags.Set(stateActive);                                  // 设置 bit 1
flags.Reset(stateNew);                                   // 清除 bit 0
bool isNone = flags.None();                              // false（bit 1 仍为 1）

// 7. 位运算
auto inverted = ~flags;                                  // 翻转有效位
auto intersected = flags & stateActive;                  // 交集
auto xored = flags ^ combined;                           // 对称差

// 8. 与 IComponent 配合
class IComponent
{
protected:
    void setState(EComponentState state, bool value)
    {
        if (value) m_stateFlags.Set(state);
        else       m_stateFlags.Reset(state);
    }
    bool isOn(EComponentState state) const
    {
        return m_stateFlags.Test(state);
    }
private:
    EComponentStateFlags m_stateFlags;
};
```

### 4.5 API 参考 — FlagEnum

| 成员 | 可见性 | 说明 |
|------|--------|------|
| `Make<Str>()` | public static | 按名称创建标志值 |
| `IsUndefined()` | public | 是否为 undefined（索引 0） |
| `operator std::string_view()` | public | 获取当前值名称 |
| `explicit operator UnderlyingType()` | public | 获取顺序索引 |
| `operator==` / `operator!=` | public friend | 相等/不等比较 |
| `operator\|` | public | `FlagEnum \| FlagEnum` → `EnumFlags`；`FlagEnum \| EnumFlags` → `EnumFlags` |

### 4.6 API 参考 — EnumFlags

#### 位运算符

| 运算符 | 操作数 | 返回 |
|--------|--------|------|
| `\|` | `EnumFlags, EnumFlags` / `EnumFlags, FlagEnum` | `EnumFlags` |
| `&` | `EnumFlags, EnumFlags` / `EnumFlags, FlagEnum` | `EnumFlags` |
| `^` | `EnumFlags, EnumFlags` | `EnumFlags` |
| `~` | `EnumFlags` | `EnumFlags`（仅翻转已注册标志位） |
| `\|=` | `EnumFlags&, EnumFlags` / `EnumFlags&, FlagEnum` | `EnumFlags&` |
| `&=` | `EnumFlags&, EnumFlags` | `EnumFlags&` |
| `^=` | `EnumFlags&, EnumFlags` | `EnumFlags&` |

#### 查询方法

| 方法 | 说明 |
|------|------|
| `Test(FlagEnum)` | 检查指定标志位是否为 1 |
| `Test(EnumFlags)` | 检查与另一 EnumFlags 是否有交集 |
| `Any()` | 是否有任意标志位为 1 |
| `None()` | 是否所有标志位均为 0 |
| `All()` | 是否所有已注册标志位均为 1 |

#### 修改方法

| 方法 | 说明 |
|------|------|
| `Set(FlagEnum)` | 设置指定标志位为 1 |
| `Reset(FlagEnum)` | 清除指定标志位为 0 |
| `Reset()` | 清除所有标志位 |

#### 转换与比较

| 成员 | 说明 |
|------|------|
| `explicit operator UnderlyingType()` | 转为整数（可能截断） |
| `explicit operator bool()` | 等价于 `Any()` |
| `ToUllong()` | 转为 `unsigned long long`（完整精度） |
| `operator==` / `operator!=` | 按位比较相等/不等 |

#### 类型特征

```cpp
flag_underlying_type_t<FlagEnum<EnumTag>>  // → EnumTag::UnderlyingType
```

#### `std::hash` 特化

`EnumFlags<EnumTag>` 已提供 `std::hash` 特化（基于 `ToUllong()`），可直接用于无序容器。

---

## 5. 两者对比

| | ExtensibleEnum | ExtensibleFlagEnum |
|---|---|---|
| **单值类型** | `Enum<EnumTag>` | `FlagEnum<EnumTag>` |
| **组合类型** | 无 | `EnumFlags<EnumTag>` |
| **存储模型** | 顺序索引 | 顺序索引 → bitset 位位置映射 |
| **`\|` 运算** | 不支持 | 支持，返回 `EnumFlags` |
| **底层存储** | `UnderlyingType m_value` | `std::bitset<MaxCount> m_bits` |
| **典型场景** | 类型区分、状态标识 | 权限标志、组件状态开关 |
| **Tag 定义** | 相同 | 相同 |
| **Class 注册** | `EnumClass` | `FlagEnumClass` |

---

## 6. 扩展与跨模块注册

同一 EnumTag 可被多个 Class 分别注册，值自动递增。这对插件式架构非常有用：

```cpp
// Core 模块
const inline EnumClass<MyTag, "Base1", "Base2"> coreEnums;

// Plugin 模块（值从 3 开始）
const inline EnumClass<MyTag, "Ext1", "Ext2", "Ext3"> pluginEnums;

// 也可跨模块扩展 FlagEnum
const inline FlagEnumClass<MyFlagTag, "CoreFlag1"> coreFlags;
const inline FlagEnumClass<MyFlagTag, "PluginFlag1", "PluginFlag2"> pluginFlags;
```

`Has()` 方法可判断某个枚举值属于哪个 Class 注册的范围。

---

## 7. 线程安全与初始化顺序

- `Names`、`Count`、`AllFlagsMask` 为 `static inline` 成员，依赖 C++17 动态初始化。
- `EnumClass` / `FlagEnumClass` 的构造函数在静态初始化阶段执行，修改上述静态成员。
- 若多个 Class 在不同翻译单元中注册同一 Tag，需注意静态初始化顺序问题（与常规 C++ 全局对象一致）。
- 建议将 Class 实例放在头文件的 `inline const` 变量中，以确保在使用前完成注册。
