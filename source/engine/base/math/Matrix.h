#pragma once
#include "Vector.h"
#include <vector>

namespace CS
{

template <typename T, int N>
class Matrix
{
public:
    Matrix()
    {
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                m_data[i * N + j] = (i == j) ? T(1) : T(0);
    }

    Matrix(const T (&values)[N][N])
    {
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                m_data[i * N + j] = values[i][j];
    }

    Matrix(const std::array<T, N * N>& arr) : m_data(arr) {}

    T* operator[](int row) { return m_data.data() + row * N; }
    const T* operator[](int row) const { return m_data.data() + row * N; }

    constexpr Vector<T, N> GetRow(uint32_t row) const
    {
        Vector<T, N> result;
        for (uint32_t index = 0u; index < N; ++index) {
            result[index] = m_mat[index][row];
        }
        return result;
    }

    constexpr void SetRow(uint32_t row, const Vector<T, N>& rowVector)
    {
        for (uint32_t index = 0u; index < N; ++index) {
            m_mat[index][row] = rowVector[index];
        }
    }

    constexpr Vector<T, N> GetColumn(uint32_t column) const
    {
        Vector<T, N> result;
        for (uint32_t index = 0u; index < N; ++index) {
            result[index] = m_mat[column][index];
        }
        return result;
    }

    constexpr void SetColumn(uint32_t column, const Vector<T, N>& columnVector)
    {
        for (uint32_t index = 0u; index < N; ++index) {
            m_mat[column][index] = columnVector[index];
        }
    }

    const std::array<T, N * N>& Data() const { return m_data; }
    std::vector<T> ToStdVector() const
    {
        auto data = m_data.data();
        return std::vector<T>(data, data + N * N);
    }

    Matrix<T, N> operator*(const Matrix<T, N>& rhs) const
    {
        Matrix<T, N> result;
        if constexpr (N == 4) {
            // Unrolled 4x4 multiply. Row-major, cache-friendly: each output row
            // is computed from a single row of *this and all rows of rhs.
            const T* a = m_data.data();
            const T* b = rhs.m_data.data();
            T* r = result.m_data.data();
            for (int i = 0; i < 4; ++i) {
                const T a0 = a[i * 4 + 0];
                const T a1 = a[i * 4 + 1];
                const T a2 = a[i * 4 + 2];
                const T a3 = a[i * 4 + 3];
                r[i * 4 + 0] = a0 * b[0] + a1 * b[4] + a2 * b[8] + a3 * b[12];
                r[i * 4 + 1] = a0 * b[1] + a1 * b[5] + a2 * b[9] + a3 * b[13];
                r[i * 4 + 2] = a0 * b[2] + a1 * b[6] + a2 * b[10] + a3 * b[14];
                r[i * 4 + 3] = a0 * b[3] + a1 * b[7] + a2 * b[11] + a3 * b[15];
            }
        } else {
            for (int i = 0; i < N; ++i) {
                for (int j = 0; j < N; ++j) {
                    T sum = T(0);
                    for (int k = 0; k < N; ++k)
                        sum += m_data[i * N + k] * rhs.m_data[k * N + j];
                    result.m_data[i * N + j] = sum;
                }
            }
        }
        return result;
    }

    Matrix<T, N>& operator*=(const Matrix<T, N>& rhs)
    {
        *this = *this * rhs;
        return *this;
    }

    Matrix<T, N> operator+(const Matrix<T, N>& rhs) const
    {
        Matrix<T, N> result;
        for (int i = 0; i < N * N; ++i)
            result.m_data[i] = m_data[i] + rhs.m_data[i];
        return result;
    }

    Matrix<T, N>& operator+=(const Matrix<T, N>& rhs)
    {
        for (int i = 0; i < N * N; ++i)
            m_data[i] += rhs.m_data[i];
        return *this;
    }

    Matrix<T, N> operator-(const Matrix<T, N>& rhs) const
    {
        Matrix<T, N> result;
        for (int i = 0; i < N * N; ++i)
            result.m_data[i] = m_data[i] - rhs.m_data[i];
        return result;
    }

    Matrix<T, N>& operator-=(const Matrix<T, N>& rhs)
    {
        for (int i = 0; i < N * N; ++i)
            m_data[i] -= rhs.m_data[i];
        return *this;
    }

    Matrix<T, N> operator*(T scalar) const
    {
        Matrix<T, N> result;
        for (int i = 0; i < N * N; ++i)
            result.m_data[i] = m_data[i] * scalar;
        return result;
    }

    Matrix<T, N>& operator*=(T scalar)
    {
        for (int i = 0; i < N * N; ++i)
            m_data[i] *= scalar;
        return *this;
    }

    Matrix<T, N> operator/(T scalar) const
    {
        Matrix<T, N> result;
        for (int i = 0; i < N * N; ++i)
            result.m_data[i] = m_data[i] / scalar;
        return result;
    }

    Matrix<T, N>& operator/=(T scalar)
    {
        for (int i = 0; i < N * N; ++i)
            m_data[i] /= scalar;
        return *this;
    }

    // Transpose
    Matrix<T, N> Transposed() const
    {
        Matrix<T, N> result;
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                result.m_data[j * N + i] = m_data[i * N + j];
        return result;
    }

    // In-place transpose
    void Transpose()
    {
        for (int i = 0; i < N; ++i)
            for (int j = i + 1; j < N; ++j)
                std::swap(m_data[i * N + j], m_data[j * N + i]);
    }

    // Inverse (only for N=2,3,4, float)
    Matrix<T, N> Inversed() const
    {
        Matrix<T, N> result;
        if constexpr (N == 2) {
            T det = m_data[0] * m_data[3] - m_data[1] * m_data[2];
            if (det == T(0))
                return result; // returns identity if not invertible
            T invDet = T(1) / det;
            result.m_data[0] = m_data[3] * invDet;
            result.m_data[1] = -m_data[1] * invDet;
            result.m_data[2] = -m_data[2] * invDet;
            result.m_data[3] = m_data[0] * invDet;
        } else if constexpr (N == 3) {
            T det = m_data[0] * (m_data[4] * m_data[8] - m_data[5] * m_data[7]) -
                    m_data[1] * (m_data[3] * m_data[8] - m_data[5] * m_data[6]) +
                    m_data[2] * (m_data[3] * m_data[7] - m_data[4] * m_data[6]);
            if (det == T(0))
                return result;
            T invDet = T(1) / det;
            result.m_data[0] = (m_data[4] * m_data[8] - m_data[5] * m_data[7]) * invDet;
            result.m_data[1] = (m_data[2] * m_data[7] - m_data[1] * m_data[8]) * invDet;
            result.m_data[2] = (m_data[1] * m_data[5] - m_data[2] * m_data[4]) * invDet;
            result.m_data[3] = (m_data[5] * m_data[6] - m_data[3] * m_data[8]) * invDet;
            result.m_data[4] = (m_data[0] * m_data[8] - m_data[2] * m_data[6]) * invDet;
            result.m_data[5] = (m_data[2] * m_data[3] - m_data[0] * m_data[5]) * invDet;
            result.m_data[6] = (m_data[3] * m_data[7] - m_data[4] * m_data[6]) * invDet;
            result.m_data[7] = (m_data[1] * m_data[6] - m_data[0] * m_data[7]) * invDet;
            result.m_data[8] = (m_data[0] * m_data[4] - m_data[1] * m_data[3]) * invDet;
        } else if constexpr (N == 4) {
            // For brevity, use a simple Gauss-Jordan elimination for 4x4
            Matrix<T, N> tmp = *this;
            result.SetIdentity();
            for (int i = 0; i < N; ++i) {
                // Find pivot
                int pivot = i;
                for (int j = i + 1; j < N; ++j)
                    if (std::abs(tmp.m_data[j * N + i]) > std::abs(tmp.m_data[pivot * N + i]))
                        pivot = j;
                if (tmp.m_data[pivot * N + i] == T(0))
                    return result;
                // Swap rows
                if (pivot != i) {
                    for (int k = 0; k < N; ++k) {
                        std::swap(tmp.m_data[i * N + k], tmp.m_data[pivot * N + k]);
                        std::swap(result.m_data[i * N + k], result.m_data[pivot * N + k]);
                    }
                }
                // Normalize row
                T div = tmp.m_data[i * N + i];
                for (int k = 0; k < N; ++k) {
                    tmp.m_data[i * N + k] /= div;
                    result.m_data[i * N + k] /= div;
                }
                // Eliminate other rows
                for (int j = 0; j < N; ++j) {
                    if (j == i)
                        continue;
                    T factor = tmp.m_data[j * N + i];
                    for (int k = 0; k < N; ++k) {
                        tmp.m_data[j * N + k] -= factor * tmp.m_data[i * N + k];
                        result.m_data[j * N + k] -= factor * result.m_data[i * N + k];
                    }
                }
            }
        } else {
            // Not implemented for other sizes
            result.SetIdentity();
        }
        return result;
    }

    // Set to identity
    void SetIdentity()
    {
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                m_data[i * N + j] = (i == j) ? T(1) : T(0);
    }

    // Zero matrix
    void SetZero()
    {
        for (int i = 0; i < N * N; ++i)
            m_data[i] = T(0);
    }

private:
    union {
        T m_mat[N][N];
        std::array<T, N * N> m_data;
    };
};

template <typename T, uint32_t N>
constexpr Vector<T, N> operator*(const Matrix<T, N>& m, const Vector<T, N>& v)
{
    Vector<T, N> result;
    for (int i = 0; i < N; ++i) {
        result[i] = T(0);
        for (int j = 0; j < N; ++j)
            result[i] += m[i][j] * v[j];
    }
    return result;
}

template <typename T, uint32_t N>
constexpr Vector<T, N - 1> operator*(const Matrix<T, N>& m, const Vector<T, N - 1>& v)
{
    Vector<T, N - 1> result;
    for (int i = 0; i < N - 1; ++i) {
        result[i] = T(0);
        for (int j = 0; j < N - 1; ++j)
            result[i] += m[i][j] * v[j];
    }
    return result;
}

// Type aliases for common dimensions
using Matrix2f = Matrix<float, 2>;
using Matrix3f = Matrix<float, 3>;
using Matrix4f = Matrix<float, 4>;

using Matrix2d = Matrix<double, 2>;
using Matrix3d = Matrix<double, 3>;
using Matrix4d = Matrix<double, 4>;

template <typename T>
using Matrix2 = Matrix<T, 2>;

template <typename T>
using Matrix3 = Matrix<T, 3>;

template <typename T>
using Matrix4 = Matrix<T, 4>;

namespace Math
{

template <typename T>
Matrix4<T> Translation(const Vector3<T>& translation)
{
    Matrix4<T> result;
    result.SetIdentity();
    result[0][3] = translation.X();
    result[1][3] = translation.Y();
    result[2][3] = translation.Z();
    return result;
}

template <typename T>
Matrix3<T> Rotation(const Vector3<T>& rotation)
{
    // Assuming rotation is in radians and using Euler angles for simplicity
    Matrix3<T> result;
    result.SetIdentity();

    T cosX = std::cos(rotation.X());
    T sinX = std::sin(rotation.X());
    T cosY = std::cos(rotation.Y());
    T sinY = std::sin(rotation.Y());
    T cosZ = std::cos(rotation.Z());
    T sinZ = std::sin(rotation.Z());

    // Rotation around X
    result[1][1] = cosX;
    result[1][2] = -sinX;
    result[2][1] = sinX;
    result[2][2] = cosX;

    // Rotation around Y
    Matrix3<T> rotY;
    rotY.SetIdentity();
    rotY[0][0] = cosY;
    rotY[0][2] = sinY;
    rotY[2][0] = -sinY;
    rotY[2][2] = cosY;

    // Rotation around Z
    Matrix3<T> rotZ;
    rotZ.SetIdentity();
    rotZ[0][0] = cosZ;
    rotZ[0][1] = -sinZ;
    rotZ[1][0] = sinZ;
    rotZ[1][1] = cosZ;

    return result * rotY * rotZ;
}

template <typename T>
Matrix4<T> RotationToMatrix4(const Vector3<T>& rotation)
{
    // Assuming rotation is in radians and using Euler angles for simplicity
    Matrix4<T> result;
    result.SetIdentity();

    T cosX = std::cos(rotation.X());
    T sinX = std::sin(rotation.X());
    T cosY = std::cos(rotation.Y());
    T sinY = std::sin(rotation.Y());
    T cosZ = std::cos(rotation.Z());
    T sinZ = std::sin(rotation.Z());

    // Rotation around X
    result[1][1] = cosX;
    result[1][2] = -sinX;
    result[2][1] = sinX;
    result[2][2] = cosX;

    // Rotation around Y
    Matrix4<T> rotY;
    rotY.SetIdentity();
    rotY[0][0] = cosY;
    rotY[0][2] = sinY;
    rotY[2][0] = -sinY;
    rotY[2][2] = cosY;

    // Rotation around Z
    Matrix4<T> rotZ;
    rotZ.SetIdentity();
    rotZ[0][0] = cosZ;
    rotZ[0][1] = -sinZ;
    rotZ[1][0] = sinZ;
    rotZ[1][1] = cosZ;

    return rotZ * rotY * result;
}

template <typename T>
Matrix3<T> Rotation(const Vector3<T>& axis, T angle)
{
    T cosA = std::cos(angle);
    T sinA = std::sin(angle);
    T oneMinusCosA = T(1) - cosA;

    Matrix3<T> result;
    result.SetIdentity();
    result[0][0] = cosA + axis.X() * axis.X() * oneMinusCosA;
    result[0][1] = axis.X() * axis.Y() * oneMinusCosA - axis.Z() * sinA;
    result[0][2] = axis.X() * axis.Z() * oneMinusCosA + axis.Y() * sinA;

    result[1][0] = axis.Y() * axis.X() * oneMinusCosA + axis.Z() * sinA;
    result[1][1] = cosA + axis.Y() * axis.Y() * oneMinusCosA;
    result[1][2] = axis.Y() * axis.Z() * oneMinusCosA - axis.X() * sinA;

    result[2][0] = axis.Z() * axis.X() * oneMinusCosA - axis.Y() * sinA;
    result[2][1] = axis.Z() * axis.Y() * oneMinusCosA + axis.X() * sinA;
    result[2][2] = cosA + axis.Z() * axis.Z() * oneMinusCosA;

    return result;
}

template <typename T>
Matrix4<T> RotationToMatrix4(const Vector3<T>& axis, T angle)
{
    // Normalize the axis
    T length = std::sqrt(axis.X() * axis.X() + axis.Y() * axis.Y() + axis.Z() * axis.Z());
    if (length == T(0))
        return Matrix4<T>();

    axis /= length;
    T cosA = std::cos(angle);
    T sinA = std::sin(angle);
    T oneMinusCosA = T(1) - cosA;

    Matrix4<T> result;
    result.SetIdentity();
    result[0][0] = cosA + axis.X() * axis.X() * oneMinusCosA;
    result[0][1] = axis.X() * axis.Y() * oneMinusCosA - axis.Z() * sinA;
    result[0][2] = axis.X() * axis.Z() * oneMinusCosA + axis.Y() * sinA;

    result[1][0] = axis.Y() * axis.X() * oneMinusCosA + axis.Z() * sinA;
    result[1][1] = cosA + axis.Y() * axis.Y() * oneMinusCosA;
    result[1][2] = axis.Y() * axis.Z() * oneMinusCosA - axis.X() * sinA;

    result[2][0] = axis.Z() * axis.X() * oneMinusCosA - axis.Y() * sinA;
    result[2][1] = axis.Z() * axis.Y() * oneMinusCosA + axis.X() * sinA;
    result[2][2] = cosA + axis.Z() * axis.Z() * oneMinusCosA;

    return result;
}

template <typename T>
Matrix3<T> Rotation(const Vector3<T>& axisX, const Vector3<T>& axisY, const Vector3<T>& axisZ)
{
    Matrix3<T> result;
    result.SetIdentity();

    result[0][0] = axisX.X();
    result[0][1] = axisY.X();
    result[0][2] = axisZ.X();

    result[1][0] = axisX.Y();
    result[1][1] = axisY.Y();
    result[1][2] = axisZ.Y();

    result[2][0] = axisX.Z();
    result[2][1] = axisY.Z();
    result[2][2] = axisZ.Z();

    return result;
}

template <typename T>
Matrix4<T> Rotation(const Vector3<T>& axisX, const Vector3<T>& axisY, const Vector3<T>& axisZ)
{
    Matrix4<T> result;
    result.SetIdentity();

    result[0][0] = axisX.X();
    result[0][1] = axisY.X();
    result[0][2] = axisZ.X();

    result[1][0] = axisX.Y();
    result[1][1] = axisY.Y();
    result[1][2] = axisZ.Y();

    result[2][0] = axisX.Z();
    result[2][1] = axisY.Z();
    result[2][2] = axisZ.Z();

    return result;
}

template <typename T>
Matrix4<T> Scaling(const Vector3<T>& scale)
{
    Matrix4<T> result;
    result.SetIdentity();
    result[0][0] = scale.X();
    result[1][1] = scale.Y();
    result[2][2] = scale.Z();
    return result;
}

template <size_t N, typename T, size_t U>
    requires(U >= N)
Matrix<T, N> Extract(const Matrix<T, U>& mat)
{
    Matrix<T, N> result;
    for (size_t i = 0; i < N; ++i)
        for (size_t j = 0; j < N; ++j)
            result[i][j] = mat[i][j];
    return result;
};

// Closed-form TRS composition: out = Translation(t) * R_zyx(euler) * Scale(s).
// Uses the same Euler convention as Math::RotationToMatrix4 (Rz * Ry * Rx).
// Avoids building three intermediate matrices and running two 4x4 multiplies,
// writing the result directly in ~20 multiplies.
template <typename T>
void ComposeTRS(Matrix4<T>& out, const Vector3<T>& translation, const Vector3<T>& eulerXYZ, const Vector3<T>& scale)
{
    const T cx = std::cos(eulerXYZ.X());
    const T sx = std::sin(eulerXYZ.X());
    const T cy = std::cos(eulerXYZ.Y());
    const T sy = std::sin(eulerXYZ.Y());
    const T cz = std::cos(eulerXYZ.Z());
    const T sz = std::sin(eulerXYZ.Z());

    // Pre-compute shared products.
    const T sx_sy = sx * sy;
    const T cx_sy = cx * sy;

    const T sX = scale.X();
    const T sY = scale.Y();
    const T sZ = scale.Z();

    // Row 0.
    out[0][0] = (cz * cy) * sX;
    out[0][1] = (cz * sx_sy - sz * cx) * sY;
    out[0][2] = (cz * cx_sy + sz * sx) * sZ;
    out[0][3] = translation.X();

    // Row 1.
    out[1][0] = (sz * cy) * sX;
    out[1][1] = (sz * sx_sy + cz * cx) * sY;
    out[1][2] = (sz * cx_sy - cz * sx) * sZ;
    out[1][3] = translation.Y();

    // Row 2.
    out[2][0] = (-sy) * sX;
    out[2][1] = (cy * sx) * sY;
    out[2][2] = (cy * cx) * sZ;
    out[2][3] = translation.Z();

    // Row 3.
    out[3][0] = T(0);
    out[3][1] = T(0);
    out[3][2] = T(0);
    out[3][3] = T(1);
}

} // namespace Math

} // namespace CS
