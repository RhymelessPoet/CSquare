#pragma once
#include "Vector.h"

namespace CS
{

template <typename T>
    requires std::is_floating_point_v<T>
class Quaternion
{
public:
    Quaternion(T x = 0, T y = 0, T z = 0, T w = 1) : x(x), y(y), z(z), w(w){};

    Quaternion(const Vector3<T>& eulerAngles)
    {
        auto [roll, pitch, yaw] = eulerAngles;
        T sinRoll = std::sin(roll * static_cast<T>(0.5));
        T cosRoll = std::cos(roll * static_cast<T>(0.5));
        T sinPitch = std::sin(pitch * static_cast<T>(0.5));
        T cosPitch = std::cos(pitch * static_cast<T>(0.5));
        T sinYaw = std::sin(yaw * static_cast<T>(0.5));
        T cosYaw = std::cos(yaw * static_cast<T>(0.5));

        x = sinRoll * cosPitch * cosYaw - cosRoll * sinPitch * sinYaw;
        y = cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw;
        z = cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw;
        w = cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw;
    }

    Quaternion(const Vector3<T>& axis, T angle)
    {
        T halfAngle = angle * static_cast<T>(0.5);
        T sinHalfAngle = std::sin(halfAngle);
        T cosHalfAngle = std::cos(halfAngle);

        x = axis.X() * sinHalfAngle;
        y = axis.Y() * sinHalfAngle;
        z = axis.Z() * sinHalfAngle;
        w = cosHalfAngle;
    }

    Quaternion operator*(const Quaternion& rhs) const
    {
        return Quaternion(w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y, // x
                          w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x, // y
                          w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w, // z
                          w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z  // w
        );
    }

    template <typename U>
        requires(std::is_arithmetic_v<U>)
    decltype(auto) operator*(const Vector3<U>& vec) const
    {
        // 纯四元数乘以向量，等价于旋转向量
        Quaternion vecQuat(vec.X(), vec.Y(), vec.Z(), 0);
        Quaternion resultQuat = (*this) * vecQuat * Conjugate();
        return Vector3<T>({resultQuat.x, resultQuat.y, resultQuat.z});
    }

    Quaternion Conjugate() const { return Quaternion(-x, -y, -z, w); }

public:
    T x;
    T y;
    T z;
    T w;
};

} // namespace CS
