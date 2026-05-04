#pragma once
#include "base/Macros.h"
#include <memory>

namespace CS
{

template <typename Derived>
struct ImplData
{
};

template <typename Derived, typename... Bases>
class PImpl : public Bases...
{
public:
    using Impl = ImplData<Derived>;
    using ImplBase = PImpl<Derived, Bases...>;

    template <typename... Args>
    PImpl(Args&&... args)
    {
        m_impl = std::make_unique<Impl>(std::forward<Args>(args)...);
    }

protected:
    ~PImpl() = default;

protected:
    Impl& impl() { return *m_impl; }
    const Impl& impl() const { return *m_impl; }

private:
    std::unique_ptr<Impl> m_impl;
};

template <typename Derived>
class PSharedImpl : public std::enable_shared_from_this<PSharedImpl<Derived>>
{
public:
    using Impl = ImplData<Derived>;
    using ImplBase = PSharedImpl<Derived>;

    template <typename... Args>
    PSharedImpl(Args&&... args)
    {
        m_impl = std::make_unique<Impl>(std::forward<Args>(args)...);
    }

protected:
    ~PSharedImpl() {}

    [[nodiscard]] std::shared_ptr<Derived> shared_from_this()
    {
        return std::dynamic_pointer_cast<Derived>(ImplBase::shared_from_this());
    }

    [[nodiscard]] std::shared_ptr<const Derived> shared_from_this() const
    {
        return std::dynamic_pointer_cast<const Derived>(ImplBase::shared_from_this());
    }

    [[nodiscard]] std::weak_ptr<Derived> weak_from_this() noexcept { return shared_from_this(); }

    [[nodiscard]] std::weak_ptr<const Derived> weak_from_this() const noexcept { return shared_from_this(); }

protected:
    Impl& impl() { return *m_impl; }
    const Impl& impl() const { return *m_impl; }

private:
    std::unique_ptr<Impl> m_impl;
};

} // namespace CS
