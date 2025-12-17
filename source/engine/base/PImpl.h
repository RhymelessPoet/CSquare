#pragma once
#include <memory>

namespace CS
{

template <typename T>
struct ImplData
{
};

template <typename T>
class PImpl
{
public:
    using Impl = ImplData<T>;
    using ImplBase = PImpl<T>;

    template <typename... Args>
    PImpl(Args&&... args)
    {
        m_impl = std::make_unique<Impl>(std::forward<Args>(args)...);
    }
    ~PImpl() {}

protected:
    Impl& impl() { return *m_impl; }
    const Impl& impl() const { return *m_impl; }

private:
    std::unique_ptr<Impl> m_impl;
};

template <typename T>
class PSharedImpl : public std::enable_shared_from_this<PSharedImpl<T>>
{
public:
    using Impl = ImplData<T>;
    using ImplBase = PSharedImpl<T>;

    template <typename... Args>
    PSharedImpl(Args&&... args)
    {
        m_impl = std::make_unique<Impl>(std::forward<Args>(args)...);
    }
    ~PSharedImpl() {}

    [[nodiscard]] std::shared_ptr<T> shared_from_this()
    {
        return std::dynamic_pointer_cast<T>(ImplBase::shared_from_this());
    }

    [[nodiscard]] std::shared_ptr<const T> shared_from_this() const
    {
        return std::dynamic_pointer_cast<const T>(ImplBase::shared_from_this());
    }

    [[nodiscard]] std::weak_ptr<T> weak_from_this() noexcept { return shared_from_this(); }

    [[nodiscard]] std::weak_ptr<const T> weak_from_this() const noexcept { return shared_from_this(); }

protected:
    Impl& impl() { return *m_impl; }
    const Impl& impl() const { return *m_impl; }

private:
    std::unique_ptr<Impl> m_impl;
};

} // namespace CS
