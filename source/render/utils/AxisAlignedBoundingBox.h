#pragma once
#include "base/math/Vector.h"
#include <array>
#include <vector>

namespace CS
{
class OrientedBoundingBox;

class AxisAlignedBoundingBox final
{
public:
    AxisAlignedBoundingBox(/* args */);
    AxisAlignedBoundingBox(const Vector3d& min, const Vector3d& max) : m_min(min), m_max(max) {}

    void SetMin(const Vector3d& min) { m_min = min; }
    void SetMax(const Vector3d& max) { m_max = max; }
    const Vector3d& GetMin() const { return m_min; }
    const Vector3d& GetMax() const { return m_max; }
    std::array<Vector3d, 8> GetCorners() const;

    Vector3d GetCenter() const { return (m_min + m_max) * 0.5; }
    Vector3d GetSize() const { return m_max - m_min; }
    double GetDiagonalLength() const { return (m_max - m_min).Length(); }

    void Include(const Vector3d& point);
    void Include(const std::vector<Vector3d>& points);

    void Include(const AxisAlignedBoundingBox& box);
    void Include(const OrientedBoundingBox& box);

    void Reset();

    bool IsValid() const;

private:
    Vector3d m_min;
    Vector3d m_max;
};

using AABB = AxisAlignedBoundingBox;

template <size_t N>
decltype(auto) get(const AABB& box)
{
    if constexpr (N == 0) {
        return box.GetMin();
    } else if constexpr (N == 1) {
        return box.GetMax();
    } else {
        static_assert(N < 2, "Index out of bounds for AxisAlignedBoundingBox");
    }
}

} // namespace CS

namespace std
{

template <>
struct tuple_size<CS::AxisAlignedBoundingBox> : std::integral_constant<size_t, 2>
{
};

template <size_t N>
struct tuple_element<N, CS::AxisAlignedBoundingBox>
{
    using type = CS::Vector3d;
};

} // namespace std
