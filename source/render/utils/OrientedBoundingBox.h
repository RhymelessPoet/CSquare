#pragma once
#include "base/Size.h"
#include "base/math/Vector.h"
#include <array>
#include <vector>

namespace CS
{

class OrientedBoundingBox final
{
public:
    OrientedBoundingBox(/* args */);
    OrientedBoundingBox(const Vector3d& center,
                        const Vector3d& axisX,
                        const Vector3d& axisY,
                        const Vector3d& axisZ,
                        const Size3d& size);

    void SetCenter(const Vector3d& center) { m_center = center; }
    void SetAxes(const Vector3d& axisX, const Vector3d& axisY, const Vector3d& axisZ);
    void SetSize(const Size3d& size) { m_halfSizes = size * 0.5; }

    const Vector3d& GetCenter() const { return m_center; }
    const std::array<Vector3d, 3>& GetAxes() const { return m_axes; }
    const Size3d& GetHalfSize() const { return m_halfSizes; }

    std::array<Vector3d, 8> GetCorners() const;

    bool IsContain(const Vector3d& point);
    bool IsContain(const std::vector<Vector3d>& points);

private:
    void check() const;

private:
    Vector3d m_center;
    std::array<Vector3d, 3> m_axes;
    Size3d m_halfSizes;
};

using OBB = OrientedBoundingBox;

} // namespace CS
