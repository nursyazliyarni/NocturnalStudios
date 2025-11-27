/*!------------------------------------------------------------------------------------------------
    File: Matrix.h
    Project: NocturnalStudio

    This file contains the declarations and implementation of a templated Matrix class and its
    specializations for 3x3 and 4x4 matrices. It includes matrix operations such as addition,
    subtraction, scalar multiplication/division, and transposition.

    Primary Author:
        Lau Xiang Yang (100%)
        - Created the Matrix class template and its specializations.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <cmath>
#include <array>
#include <iomanip>
#include <type_traits>

#include "../glm/glm/matrix.hpp"
#include "../include/math/Vector.h"
#include "../include/core/Logger.h"

constexpr double PI = 3.14159265358979323846;

using namespace std;
extern Logger logger;

template <size_t N>
class Vector;

//********* Matrix Class Template Declaration **********//

/**
 * @brief Fixed-size square matrix template.
 *
 * Template parameter N defines both width and height (N x N).
 * Designed for small matrices (3x3, 4x4) used in 2D/3D transforms.
 *
 * Notes:
 * - Data is stored row-major in a std::array<float, N*N>.
 * - Provides common arithmetic, row access, transpose, inverse and conversion helpers.
 */
template <size_t N>
class Matrix {
public:
    /** @name Constructors / Destructor */
    /**@{*/
    /**
     * @brief Default-constructs a zero-initialized matrix.
     */
    Matrix();

    /**
     * @brief Construct from an initializer list.
     * @param list Must contain exactly N*N floats in row-major order.
     * If size mismatches, a logger error is produced and matrix is left unchanged.
     */
    Matrix(std::initializer_list<float> list);

    /**
     * @brief Copy constructor.
     * @param other Matrix to copy.
     */
    Matrix(const Matrix& other);

    /**
     * @brief Construct from glm::mat3x3 (only when N==3).
     * @param other GLM matrix to convert from.
     */
    Matrix(glm::mat3x3 other) requires(N == 3);

    /**
     * @brief Construct from glm::mat4x4 (only when N==4).
     * @param other GLM matrix to convert from.
     */
    Matrix(glm::mat4x4 other) requires(N == 4);

    /**
     * @brief Convert to glm::mat3x3 (only when N==3).
     */
    operator glm::mat3x3() const;

    /**
     * @brief Convert to glm::mat4x4 (only when N==4).
     */
    operator glm::mat4x4() const;

    /**
     * @brief Default destructor.
     */
    ~Matrix() = default;
    /**@}*/

    /** @name Assignment / Arithmetic */
    /**@{*/
    /**
     * @brief Copy-assignment.
     * @return reference to *this
     */
    Matrix& operator=(const Matrix& other) noexcept;

    /**
     * @brief Element-wise addition.
     * @return New matrix equal to (*this + other).
     */
    Matrix operator+(const Matrix& other) const noexcept;

    /**
     * @brief Matrix multiplication (this * other).
     * @return New matrix equal to the product.
     */
    Matrix operator*(const Matrix& other) const noexcept;

    /**
     * @brief Matrix-vector multiplication: transforms a Vector<N>.
     * @param vec Vector to multiply (interpreted as column vector).
     * @return Transformed Vector<N>.
     */
    Vector<N> operator*(const Vector<N>& vec) const noexcept;

    /**
     * @brief In-place addition.
     */
    Matrix& operator+=(const Matrix& other) noexcept;

    /**
     * @brief In-place subtraction.
     */
    Matrix& operator-=(const Matrix& other) noexcept;

    /**
     * @brief In-place scalar multiplication.
     */
    Matrix& operator*=(float scalar) noexcept;

    /**
     * @brief In-place scalar division.
     * Logs an error and leaves matrix unchanged if scalar == 0.
     */
    Matrix& operator/=(float scalar);

    /**
     * @brief Unary negation.
     * @return Matrix with each element negated.
     */
    Matrix operator-() const noexcept;
    /**@}*/

    /** @name Transform helpers */
    /**@{*/
    /**
     * @brief Transpose this matrix in-place.
     * @return *this (transposed).
     */
    Matrix& Transpose() noexcept;

    /**
     * @brief In-place matrix inverse.
     *
     * - For N==4, detects the common affine case (last row [0 0 0 1]) and
     *   applies a fast inverse for orthonormal rotation + translation.
     * - Otherwise uses Gauss-Jordan elimination with partial pivoting.
     *
     * On singular matrices logs an error and leaves the matrix unchanged.
     *
     * @return *this (inverted) when successful or unchanged on failure.
     */
    Matrix& Inverse() noexcept;

    /**
     * @brief Set matrix to identity.
     * @return *this (identity).
     */
    Matrix& Identity() noexcept;
    /**@}*/

    /** @name Element access */
    /**@{*/
    /**
     * @brief Return element at 1-based row/column (existing code uses 1-based indexing).
     * @param row 1-based row index in range [1..N].
     * @param col 1-based column index in range [1..N].
     * @return element value or 0.0f and logs on out-of-range inputs.
     */
    float getElement(size_t row, size_t col) const;
    /**@}*/

    /**
     * @brief Lightweight proxy for row access so mat[row][col] works.
     */
    class rowProxy {
    public:
        // Constructor
        rowProxy(float* rowData) : rowData(rowData) {}

        // Access operator
        float& operator[](size_t col) {
            return rowData[col];
        }

        const float& operator[](size_t col) const {
            return rowData[col];
        }

    private:
        float* rowData;
    };

    /**
     * @brief Return a proxy for row access (0-based row index).
     */
    rowProxy operator[](size_t row) {
        return rowProxy(&data[row * N]);
    }

    const rowProxy operator[](size_t row) const {
        return rowProxy(const_cast<float*>(&data[row * N]));
    }

    /**
     * @brief Stream output helper (nicely prints matrix rows).
     */
    friend std::ostream& operator<<(std::ostream& os, const Matrix<N>& mat) {
        for (size_t i{}; i < N; ++i) {
            for (size_t j{}; j < N; ++j) {
                os << std::setw(5) << mat.data[i * N + j] << " ";
            }
            os << std::endl;
        }
        return os;
    }

    /**
     * @brief Provide raw pointer to contiguous float data.
     * Useful for APIs that accept float*.
     */
    operator float* () {
        return data.data();
    }

public:
    /** Row-major storage of N*N floats. */
    std::array<float, N * N> data;
};

//********* Non-Member Functions **********//

/**
 * @brief Apply a 2D translation to a 3x3 transform matrix (row-major).
 * @param transformMatModel Left operand transform.
 * @param x translation in x.
 * @param y translation in y.
 * @return New transform matrix after applying translation.
 */
Matrix<3> Translate3x3(const Matrix<3>& transformMatModel, float x, float y);

/**
 * @brief Apply a 2D rotation (radians) to a 3x3 transform matrix.
 */
Matrix<3> RotateRad3x3(const Matrix<3>& transformMatModel, float radAngle);

/**
 * @brief Apply a 2D rotation (degrees) to a 3x3 transform matrix.
 */
Matrix<3> RotateDeg3x3(const Matrix<3>& transformMatModel, float degAngle);

/**
 * @brief Apply a 2D scale to a 3x3 transform matrix.
 */
Matrix<3> Scale3x3(const Matrix<3>& transformMatModel, float x, float y);

/**
 * @brief Apply a translation to a 4x4 transform matrix.
 */
Matrix<4> Translate4x4(const Matrix<4>& transformMatModel, float x, float y, float z);

/**
 * @brief Apply a rotation (radians) to a 4x4 transform matrix around Z axis.
 */
Matrix<4> RotateRad4x4(const Matrix<4>& transformMatModel, float radAngle);

/**
 * @brief Apply a rotation (degrees) to a 4x4 transform matrix around Z axis.
 */
Matrix<4> RotateDeg4x4(const Matrix<4>& transformMatModel, float degAngle);

/**
 * @brief Apply a scale to a 4x4 transform matrix.
 */
Matrix<4> Scale4x4(const Matrix<4>& transformMatModel, float x, float y, float z);

//********* Matrix Class Template Implementation **********//

// Constructors
template <size_t N>
Matrix<N>::Matrix() : data{} {};

// Copy constructor
template <size_t N>
Matrix<N>::Matrix(const Matrix<N>& other) : data(other.data) {}

// Constructor using initializer list
template <size_t N>
Matrix<N>::Matrix(std::initializer_list<float> list) {
    if (list.size() != N * N) {
        logger.error("Matrix initializer list size does not match matrix dimensions.");
        return;
    }
    std::copy(list.begin(), list.end(), data.begin());
}

// Copy assignment operator
template <size_t N>
Matrix<N>& Matrix<N>::operator=(const Matrix<N>& other) noexcept {
    if (this != &other)
        data = other.data;
    return *this;
}

// Addition operator
template <size_t N>
Matrix<N> Matrix<N>::operator+(const Matrix<N>& other) const noexcept {
    Matrix<N> result;
    for (size_t i{}; i < N * N; ++i)
        result.data[i] = data[i] + other.data[i];
    return result;
}

// Multiplication operator
template <size_t N>
Matrix<N> Matrix<N>::operator*(const Matrix<N>& other) const noexcept {
    Matrix<N> result;
    for (size_t i{}; i < N; ++i) {
        for (size_t j{}; j < N; ++j) {
            for (size_t k{}; k < N; ++k) {
                result[i][j] += (*this)[i][k] * other[k][j];
            }
        }
    }
    return result;
}

// Matrix-Vector multiplication
template <size_t N>
Vector<N> Matrix<N>::operator*(const Vector<N>& vec) const noexcept {
    Vector<N> result;
    for (size_t r = 0; r < N; ++r) {
        float acc = 0.f;
        for (size_t c = 0; c < N; ++c)
            acc = std::fma(data[r * N + c], vec[c], acc);
        result[r] = acc;
    }
    return result;
}

// Addition assignment operator
template <size_t N>
Matrix<N>& Matrix<N>::operator+=(const Matrix<N>& other) noexcept {
    for (size_t i{}; i < N * N; ++i)
        data[i] += other.data[i];
    return *this;
}

// Subtraction assignment operator
template <size_t N>
Matrix<N>& Matrix<N>::operator-=(const Matrix<N>& other) noexcept {
    for (size_t i{}; i < N * N; ++i)
        data[i] -= other.data[i];
    return *this;
}

// Multiplication assignment operator
template <size_t N>
Matrix<N>& Matrix<N>::operator*=(float scalar) noexcept {
    for (size_t i{}; i < N * N; ++i)
        data[i] *= scalar;
    return *this;
}

// Division assignment operator
template <size_t N>
Matrix<N>& Matrix<N>::operator/=(float scalar) {
    if (scalar == 0.0f) {
        logger.error("Division by zero in matrix division assignment.");
        return *this;
    }
    for (size_t i{}; i < N * N; ++i)
        data[i] /= scalar;
    return *this;
}

// Unary negation operator
template <size_t N>
Matrix<N> Matrix<N>::operator-() const noexcept {
    Matrix<N> result;
    for (size_t i{}; i < N * N; ++i)
        result.data[i] = -data[i];
    return result;
}

// Assignment operator for glm::mat3x3
template <size_t N>
inline Matrix<N>::Matrix(glm::mat3x3 other) requires(N == 3) {
    for (glm::length_t i = 0; i < N; ++i) {
        for (glm::length_t j = 0; j < N; ++j) {
            (*this)[i][j] = static_cast<float>(other[i][j]);
        }
    }
}

// Conversion operator to glm::mat3x3
template <>
inline Matrix<3>::operator glm::mat3x3() const {
    glm::mat3x3 result{ 0.f };
    for (glm::length_t i = 0; i < 3; ++i) {
        for (glm::length_t j = 0; j < 3; ++j) {
            result[static_cast<glm::mat3x3::length_type>(i)][static_cast<glm::mat3x3::length_type>(j)] = (*this)[i][j];
        }
    }
    return result;
}

// Assignment operator for glm::mat4x4
template <size_t N>
inline Matrix<N>::Matrix(glm::mat4x4 other) requires(N == 4) {
    for (glm::length_t i = 0; i < N; ++i) {
        for (glm::length_t j = 0; j < N; ++j) {
            (*this)[i][j] = static_cast<float>(other[i][j]);
        }
    }
}

// Conversion operator to glm::mat4x4
template <>
inline Matrix<4>::operator glm::mat4x4() const {
    glm::mat4x4 result{ 0.f };
    for (glm::length_t i = 0; i < 4; ++i) {
        for (glm::length_t j = 0; j < 4; ++j) {
            result[static_cast<glm::mat4x4::length_type>(i)][static_cast<glm::mat4x4::length_type>(j)] = (*this)[i][j];
        }
    }
    return result;
}

// Accessor for getting an element
template <size_t N>
float Matrix<N>::getElement(size_t row, size_t col) const {
    if (row > N || col > N) {
        logger.error("Matrix index out of bounds in getElement.");
        return 0.f;
    }

    row -= 1;
    col -= 1;
    return data[row * N + col];
}

// Identity matrix
template <size_t N>
Matrix<N>& Matrix<N>::Identity() noexcept {
    data.fill(0.f);
    for (size_t i = 0; i < N; ++i)
        data[i * N + i] = 1.f;
    return *this;
}

// Transpose matrix
template <size_t N>
Matrix<N>& Matrix<N>::Transpose() noexcept {
    Matrix<N> temp = *this;
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            (*this)[i][j] = temp[j][i];
        }
    }
    return *this;
}

// Inverse (generic): Gauss–Jordan with partial pivoting.
// Has an affine fast path for 4x4 matrices with last row [0 0 0 1].
template <size_t N>
inline Matrix<N>& Matrix<N>::Inverse() noexcept {
    constexpr float eps = 1e-6f;

    if constexpr (N == 4) {
        // Fast path for affine transforms (last row ~ [0,0,0,1])
        if (std::fabs((*this)[3][0]) < eps && std::fabs((*this)[3][1]) < eps &&
            std::fabs((*this)[3][2]) < eps && std::fabs((*this)[3][3] - 1.f) < eps) {

            // Extract R (upper-left 3x3) and t (translation)
            const float r00 = (*this)[0][0], r01 = (*this)[0][1], r02 = (*this)[0][2];
            const float r10 = (*this)[1][0], r11 = (*this)[1][1], r12 = (*this)[1][2];
            const float r20 = (*this)[2][0], r21 = (*this)[2][1], r22 = (*this)[2][2];
            const float tx  = (*this)[0][3], ty  = (*this)[1][3], tz  = (*this)[2][3];

            // If R is (near) orthonormal, inverse is [R^T | -R^T t; 0 0 0 1]
            const float c0 = r00*r00 + r10*r10 + r20*r20;
            const float c1 = r01*r01 + r11*r11 + r21*r21;
            const float c2 = r02*r02 + r12*r12 + r22*r22;
            if (std::fabs(c0 - 1.f) < 1e-3f && std::fabs(c1 - 1.f) < 1e-3f && std::fabs(c2 - 1.f) < 1e-3f) {
                // R^T
                (*this)[0][0] = r00; (*this)[0][1] = r10; (*this)[0][2] = r20;
                (*this)[1][0] = r01; (*this)[1][1] = r11; (*this)[1][2] = r21;
                (*this)[2][0] = r02; (*this)[2][1] = r12; (*this)[2][2] = r22;
                // -R^T * t
                (*this)[0][3] = -(r00*tx + r10*ty + r20*tz);
                (*this)[1][3] = -(r01*tx + r11*ty + r21*tz);
                (*this)[2][3] = -(r02*tx + r12*ty + r22*tz);
                // last row
                (*this)[3][0] = 0.f; (*this)[3][1] = 0.f; (*this)[3][2] = 0.f; (*this)[3][3] = 1.f;
                return *this;
            }
            // Fall through to generic if not orthonormal (e.g., scaling/shear present)
        }
    }

    Matrix<N> a = *this;
    Matrix<N> inv; inv.Identity();

    for (size_t i = 0; i < N; ++i) {
        // Find pivot row
        size_t pivot = i;
        float maxAbs = std::fabs(a[pivot][i]);
        for (size_t r = i + 1; r < N; ++r) {
            float v = std::fabs(a[r][i]);
            if (v > maxAbs) { maxAbs = v; pivot = r; }
        }

        if (maxAbs <= eps) {
            logger.error("Matrix is singular or ill-conditioned and cannot be inverted.");
            return *this;
        }

        // Swap to put pivot on diagonal
        if (pivot != i) {
            for (size_t c = 0; c < N; ++c) {
                std::swap(a[i][c],   a[pivot][c]);
                std::swap(inv[i][c], inv[pivot][c]);
            }
        }

        // Normalize pivot row
        const float piv = a[i][i];
        const float invPiv = 1.0f / piv;
        for (size_t c = 0; c < N; ++c) {
            a[i][c]   *= invPiv;
            inv[i][c] *= invPiv;
        }

        // Eliminate column i in all other rows
        for (size_t r = 0; r < N; ++r) {
            if (r == i) continue;
            const float f = a[r][i];
            if (f == 0.f) continue;
            for (size_t c = 0; c < N; ++c) {
                a[r][c]   -= f * a[i][c];
                inv[r][c] -= f * inv[i][c];
            }
        }
    }

    *this = inv;
    return *this;
}

#endif // MATRIX_H