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
    std::array<Vector3d, 8> corners;

    corners[0] = Vector3d{m_min.X(), m_min.Y(), m_min.Z()};
    corners[1] = Vector3d{m_min.X(), m_min.Y(), m_max.Z()};
    corners[2] = Vector3d{m_min.X(), m_max.Y(), m_min.Z()};
    corners[3] = Vector3d{m_min.X(), m_max.Y(), m_max.Z()};
    corners[4] = Vector3d{m_max.X(), m_min.Y(), m_min.Z()};
    corners[5] = Vector3d{m_max.X(), m_min.Y(), m_max.Z()};
    corners[6] = Vector3d{m_max.X(), m_max.Y(), m_min.Z()};
    corners[7] = Vector3d{m_max.X(), m_max.Y(), m_max.Z()};

    return corners;
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
