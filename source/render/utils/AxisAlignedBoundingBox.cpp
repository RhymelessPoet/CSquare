#include "AxisAlignedBoundingBox.h"
#include "OrientedBoundingBox.h"

namespace CS
{
AxisAlignedBoundingBox::AxisAlignedBoundingBox()
{
    Reset();
}

std::array<Vector3d, 8> AxisAlignedBoundingBox::GetCorners() const
{
    return std::array<Vector3d, 8>();
}

inline void AxisAlignedBoundingBox::Include(const Vector3d& point)
{
    m_min = Min(m_min, point);
    m_max = Max(m_max, point);
}

void AxisAlignedBoundingBox::Include(const std::vector<Vector3d>& points)
{
    for (const auto& point : points) {
        Include(point);
    }
}

void AxisAlignedBoundingBox::Include(const AxisAlignedBoundingBox& box)
{
    Include(box.GetMin());
    Include(box.GetMax());
}

void AxisAlignedBoundingBox::Include(const OrientedBoundingBox& box)
{
    auto corners = box.GetCorners();
    for (const auto& corner : corners) {
        Include(corner);
    }
}

void AxisAlignedBoundingBox::Reset()
{
    m_min.Fill(std::numeric_limits<double>::max());
    m_max.Fill(std::numeric_limits<double>::lowest());
}

bool AxisAlignedBoundingBox::IsValid() const
{
    return m_min.X() < m_max.X() && m_min.Y() < m_max.Y() && m_min.Z() < m_max.Z();
}

} // namespace CS
