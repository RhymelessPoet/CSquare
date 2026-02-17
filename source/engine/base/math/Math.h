#pragma once
#include "Matrix.h"
#include "Quaternion.h"
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

template <typename T, typename U>
    requires(std::is_floating_point_v<T>) && (std::is_floating_point_v<U>)
std::array<Vector3<T>, 3> EulerAnglesToAxes(const Vector3<U>& angles)
{
    auto [roll, pitch, yaw] = angles;
    T sinRoll = std::sin(roll);
    T cosRoll = std::cos(roll);
    T sinPitch = std::sin(pitch);
    T cosPitch = std::cos(pitch);
    T sinYaw = std::sin(yaw);
    T cosYaw = std::cos(yaw);

    return {Vector3<T>({cosPitch * cosYaw, cosPitch * sinYaw, -sinPitch}),
            Vector3<T>({sinRoll * sinPitch * cosYaw - cosRoll * sinYaw, sinRoll * sinPitch * sinYaw + cosRoll * cosYaw,
                        sinRoll * cosPitch}),
            Vector3<T>({cosRoll * sinPitch * cosYaw + sinRoll * sinYaw, cosRoll * sinPitch * sinYaw - sinRoll * cosYaw,
                        cosRoll * cosPitch})};
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
constexpr inline T AngleToRadian(T angle)
{
    return angle / 180.0f * PI<T>;
}

template <typename T, typename U>
Vector3<T> Transform(const Matrix4<T>& mat, const Vector3<U>& point)
{
    return mat * point.Cast<T>() + Vector3<T>({mat[0][3], mat[1][3], mat[2][3]});
}

template <typename T>
    requires std::is_floating_point_v<T>
Vector3<T> ToEulerAnglesXYZ(const Quaternion<T>& q)
{
    // xyz顺序: roll (X), pitch (Y), yaw (Z)
    T sinRollCosPitch = 2.0f * (q.w * q.x + q.y * q.z);
    T cosRollCosPitch = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    T roll = std::atan2(sinRollCosPitch, cosRollCosPitch);

    T sinPitch = 2.0f * (q.w * q.y - q.z * q.x);
    T pitch;
    if (std::abs(sinPitch) >= 1.0f) {
        pitch = std::copysign(static_cast<T>(Math::PI<T> / 2), sinPitch); // use 90 degrees if out of range
    } else {
        pitch = std::asin(sinPitch);
    }

    T sinYawCosPitch = 2.0f * (q.w * q.z + q.x * q.y);
    T cosYawCosPitch = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
    T yaw = std::atan2(sinYawCosPitch, cosYawCosPitch);

    return Vector3<T>({roll, pitch, yaw});
}

} // namespace Math

} // namespace CS
