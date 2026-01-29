#include "OrientedBoundingBox.h"

namespace CS
{
OrientedBoundingBox::OrientedBoundingBox()
    : m_center(Vector3d{0.0, 0.0, 0.0}),
      m_axes{Vector3d{1.0, 0.0, 0.0}, Vector3d{0.0, 1.0, 0.0}, Vector3d{0.0, 0.0, 1.0}},
      m_halfSizes(Size3d{1.0, 1.0, 1.0})
{}

OrientedBoundingBox::OrientedBoundingBox(const Vector3d& center,
                                         const Vector3d& axisX,
                                         const Vector3d& axisY,
                                         const Vector3d& axisZ,
                                         const Size3d& size)
    : m_center(center), m_axes{axisX.Normalized(), axisY.Normalized(), axisZ.Normalized()}, m_halfSizes(size * 0.5)
{}

void OrientedBoundingBox::SetAxes(const Vector3d& axisX, const Vector3d& axisY, const Vector3d& axisZ)
{
    m_axes[0] = axisX.Normalized();
    m_axes[1] = axisY.Normalized();
    m_axes[2] = axisZ.Normalized();
    check();
}

std::array<Vector3d, 8> OrientedBoundingBox::GetCorners() const
{
    std::array<Vector3d, 8> corners;
    corners[0] = m_center - m_axes[0] * m_halfSizes.x - m_axes[1] * m_halfSizes.y - m_axes[2] * m_halfSizes.z;
    corners[1] = m_center + m_axes[0] * m_halfSizes.x - m_axes[1] * m_halfSizes.y - m_axes[2] * m_halfSizes.z;
    corners[2] = m_center - m_axes[0] * m_halfSizes.x + m_axes[1] * m_halfSizes.y - m_axes[2] * m_halfSizes.z;
    corners[3] = m_center + m_axes[0] * m_halfSizes.x + m_axes[1] * m_halfSizes.y - m_axes[2] * m_halfSizes.z;
    corners[4] = m_center - m_axes[0] * m_halfSizes.x - m_axes[1] * m_halfSizes.y + m_axes[2] * m_halfSizes.z;
    corners[5] = m_center + m_axes[0] * m_halfSizes.x - m_axes[1] * m_halfSizes.y + m_axes[2] * m_halfSizes.z;
    corners[6] = m_center - m_axes[0] * m_halfSizes.x + m_axes[1] * m_halfSizes.y + m_axes[2] * m_halfSizes.z;
    corners[7] = m_center + m_axes[0] * m_halfSizes.x + m_axes[1] * m_halfSizes.y + m_axes[2] * m_halfSizes.z;
    return corners;
}

bool OrientedBoundingBox::IsContain(const Vector3d& point)
{
    auto direction = point - m_center;
    for (int i = 0; i < 3; ++i) {
        double distance = direction.Dot(m_axes[i]);
        if (std::abs(distance) > m_halfSizes.data[i]) {
            return false;
        }
    }
    return true;
}

bool OrientedBoundingBox::IsContain(const std::vector<Vector3d>& points)
{
    for (const auto& point : points) {
        if (!IsContain(point)) {
            return false;
        }
    }
    return true;
}

void OrientedBoundingBox::check() const
{
    // Check if the axes are orthogonal
    double dotXY = m_axes[0].Dot(m_axes[1]);
    double dotXZ = m_axes[0].Dot(m_axes[2]);
    double dotYZ = m_axes[1].Dot(m_axes[2]);
    const double epsilon = 1e-6;
    if (std::abs(dotXY) > epsilon || std::abs(dotXZ) > epsilon || std::abs(dotYZ) > epsilon) {
        // TODO: log error
    }
}

} // namespace CS
