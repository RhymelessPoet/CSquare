#pragma once
#include "Matrix.h"
#include "Vector.h"
#include <cmath>

namespace CS
{

namespace Math
{

template <typename T>
    requires(std::is_floating_point_v<T>)
static inline constexpr T PI = static_cast<T>(3.14159265358979323846);

template <typename T>
    requires(std::is_floating_point_v<T>)
Vector3<T> Vector3FromSpherical(T radius, T theta, T phi)
{
    T sinTheta = std::sin(theta);
    T cosTheta = std::cos(theta);
    T sinPhi = std::sin(phi);
    T cosPhi = std::cos(phi);

    return Vector3<T>({radius * sinTheta * cosPhi, radius * sinTheta * sinPhi, radius * cosTheta});
}

template <typename T>
    requires(std::is_floating_point_v<T>)
Vector3<T> AxesToEulerAnglesXYZ(const Vector3<T>& xAxis, const Vector3<T>& yAxis, const Vector3<T>& zAxis)
{
    // xyz顺序: roll (X), pitch (Y), yaw (Z)
    /*
    /*                   | r11 r12 r13 |   | xAxis.x yAxis.x zAxis.x |
    * Rotation Matrix =  | r21 r22 r23 | = | xAxis.y yAxis.y zAxis.y |
    /*                   | r31 r32 r33 |   | xAxis.z yAxis.z zAxis.z |
    */
    auto [r11, r21, r31] = xAxis;
    auto [r12, r22, r32] = yAxis;
    auto [r13, r23, r33] = zAxis;

    constexpr auto halfPI = PI<T> * 0.5;

    auto pitch = std::atan2(-r31, std::sqrt(r11 * r11 + r21 * r21));
    auto cosPitch = std::cos(pitch);

    if (cosPitch <= std::numeric_limits<T>::epsilon()) {
        auto roll = pitch < 0 ? -std::atan2(r12, r22) : std::atan2(r12, r22);
        return Vector3<T>({roll, pitch, 0.0f});

    } else {
        T yaw = std::atan2(r21 / cosPitch, r11 / cosPitch);
        T roll = std::atan2(r32 / cosPitch, r33 / cosPitch);

        return Vector3<T>({roll, pitch, yaw});
    }
}

template <typename T>
    requires(std::is_floating_point_v<T>)
Vector3<T> RotationMatrixToAnglesXYZ(const Matrix4<T>& rotation)
{
    auto xAxis = rotation.GetRow(0u);
    auto yAxis = rotation.GetRow(1u);
    auto zAxis = rotation.GetRow(2u);

    return AxesToEulerAnglesXYZ(Vector3<T>{xAxis[0], xAxis[1], xAxis[2]}, Vector3<T>{yAxis[0], yAxis[1], yAxis[2]},
                                Vector3<T>{zAxis[0], zAxis[1], zAxis[2]});
}

template <typename T>
    requires std::is_floating_point_v<T>
constexpr inline T AnglesToRadians(T angle)
{
    return angle / 180.0f * PI<T>;
}

} // namespace Math

} // namespace CS
