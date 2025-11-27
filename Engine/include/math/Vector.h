/*!------------------------------------------------------------------------------------------------
    File: Vector.h
    Project: NocturnalStudio

    This file contains the declarations and implementation of a templated Vector class and its
    specializations for 2D, 3D and 4D vectors. It includes vector arithmetic operations, accessors,
    mutators and useful vector functions like dot product, cross product magnitude, distance and
    normalization.

    Primary Author:
        Lau Xiang Yang (100%)
        - Created the Vector class template and its specializations.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#ifndef VECTOR_H
#define VECTOR_H

#include <iostream>
#include <cmath>

#include "../glm/glm/vec2.hpp"
#include "../glm/glm/vec3.hpp"
#include "../glm/glm/vec4.hpp"
#include "../include/core/logger.h"

using namespace std;

extern Logger logger;

template <size_t N>
class Matrix;

//********* Vector Class Template Declaration **********//

/**
 * @brief Small fixed-size vector template.
 *
 * Template parameter N defines the number of components.
 * Designed for 2D/3D/4D math used across the engine.
 *
 * Storage: float data[N]
 *
 * Provides:
 *  - basic arithmetic (+=, -=, *=, /=),
 *  - dot / cross utilities,
 *  - length / normalization helpers,
 *  - GLM interop constructors/conversions.
 */
template <size_t N>
class Vector {
public:
    using value_type = float;
    static constexpr size_t size = N;

    /**
     * @brief Default constructs a zero-initialized vector.
     */
    constexpr Vector() noexcept : data{} {}

    /**
     * @brief Default copy constructor.
     */
    constexpr Vector(const Vector& other) noexcept = default;

    /**
     * @brief Default copy-assignment.
     */
    constexpr Vector& operator=(const Vector& other) noexcept = default;

    /**
     * @brief Variadic constructor for direct component initialization.
     * @tparam Args Types convertible to float.
     * @param args Exactly N arguments are required.
     *
     * Example: Vector<3> v(1.f, 2.f, 3.f);
     */
    template <typename... Args>
        requires (sizeof...(Args) == N && (std::is_convertible_v<Args,float> && ...))
    constexpr explicit Vector(Args... args) noexcept : data{ static_cast<float>(args)... } {}

    /**
     * @brief Construct from a GLM vector of matching length (2,3 or 4).
     */
    template <glm::length_t L>
        requires (L == N && (N == 2 || N == 3 || N == 4))
    explicit constexpr Vector(glm::vec<L, float, glm::defaultp> const& v) noexcept {
        for (size_t i = 0; i < N; ++i) data[i] = v[i];
    }

    /**
     * @brief Convert to a GLM vector of matching length.
     */
    template <glm::length_t L>
        requires (L == N && (N == 2 || N == 3 || N == 4))
    constexpr operator glm::vec<L, float, glm::defaultp>() const noexcept {
        if constexpr (N == 2) return glm::vec2{ data[0], data[1] };
        else if constexpr (N == 3) return glm::vec3{ data[0], data[1], data[2] };
        else return glm::vec4{ data[0], data[1], data[2], data[3] };
    }

    // Element access -------------------------------------------------------

    /**
     * @brief Index operator (mutable).
     * @param i zero-based component index [0..N-1].
     */
    constexpr float& operator[](size_t i) noexcept { return data[i]; }

    /**
     * @brief Index operator (const).
     * @param i zero-based component index [0..N-1].
     */
    constexpr const float& operator[](size_t i) const noexcept { return data[i]; }

    // Arithmetic operators -----------------------------------------------

    /**
     * @brief In-place addition.
     */
    constexpr Vector& operator+=(const Vector& other) noexcept {
        for (size_t i=0;i<N;++i) data[i] += other.data[i];
        return *this;
    }

    /**
     * @brief In-place subtraction.
     */
    constexpr Vector& operator-=(const Vector& other) noexcept {
        for (size_t i=0;i<N;++i) data[i] -= other.data[i];
        return *this;
    }

    /**
     * @brief In-place scalar multiply.
     */
    constexpr Vector& operator*=(float s) noexcept {
        for (auto& v : data) v *= s;
        return *this;
    }

    /**
     * @brief In-place scalar divide (caller must avoid divide-by-zero).
     */
    constexpr Vector& operator/=(float s) noexcept {
        for (auto& v : data) v /= s;
        return *this;
    }

    /**
     * @brief Binary addition (returns new vector).
     */
    constexpr Vector operator+(const Vector& other) const noexcept {
        Vector r(*this); r += other; return r;
    }

    /**
     * @brief Binary subtraction (returns new vector).
     */
    constexpr Vector operator-(const Vector& other) const noexcept {
        Vector r(*this); r -= other; return r;
    }

    /**
     * @brief Unary negation.
     */
    constexpr Vector operator-() const noexcept {
        Vector r;
        for (size_t i=0;i<N;++i) r.data[i] = -data[i];
        return r;
    }

    /**
     * @brief Equality comparison (component-wise).
     */
    constexpr bool operator==(const Vector& other) const noexcept {
        for (size_t i=0;i<N;++i) if (data[i] != other.data[i]) return false;
        return true;
    }

    // Dot / Cross / Length -----------------------------------------------

    /**
     * @brief Dot product (this · other).
     * @return scalar dot product.
     */
    constexpr float dot(const Vector& other) const noexcept {
        float acc = 0.f;
        for (size_t i=0;i<N;++i) acc = std::fma(data[i], other.data[i], acc);
        return acc;
    }

    /**
     * @brief Cross product.
     *
     * - For N==3 returns Vector<3> (standard 3D cross).
     * - For N==2 returns scalar z-component magnitude (2D cross).
     * - For other N triggers static_assert.
     */
    constexpr auto cross(const Vector& other) const noexcept {
        if constexpr (N == 3) {
            return Vector<3>{
                data[1] * other.data[2] - data[2] * other.data[1],
                data[2] * other.data[0] - data[0] * other.data[2],
                data[0] * other.data[1] - data[1] * other.data[0]
            };
        } else if constexpr (N == 2) {
            return data[0] * other.data[1] - data[1] * other.data[0];
        } else {
            static_assert(N == 2 || N == 3, "cross only defined for 2D (magnitude) or 3D");
        }
    }

    /**
     * @brief Squared length (dot with self). Fast when exact length not required.
     */
    constexpr float length2() const noexcept { return dot(*this); }

    /**
     * @brief Euclidean length (magnitude).
     */
    float length() const noexcept { return std::sqrt(length2()); }

    /**
     * @brief Return normalized copy (unit length). If zero-length returns original.
     */
    Vector normalized() const noexcept {
        float len = length();
        return len > 0.f ? (*this) / len : *this;
    }

    // Friend helpers -----------------------------------------------------

    friend constexpr Vector operator*(Vector v, float s) noexcept { v *= s; return v; }
    friend constexpr Vector operator*(float s, Vector v) noexcept { v *= s; return v; }
    friend constexpr Vector operator/(Vector v, float s) noexcept { v /= s; return v; }

    // Raw storage --------------------------------------------------------

    /**
     * @brief Underlying component storage (contiguous).
     */
    float data[N];
};

// Concrete vector types -----------------------------------------------------

/**
 * @brief 2D vector convenience wrapper with named accessors.
 */
class Vector2D : public Vector<2> {
public:
    /** Default ctor (zero vector). */
    Vector2D();
    /** Construct with x,y. */
    Vector2D(float x, float y);
    /** Copy from generic Vector<2>. */
    Vector2D(const Vector<2>& other);

    /** Convert to glm::vec2. */
    operator glm::vec2() const;
    /** Assign from glm::vec2. */
    Vector2D& operator=(const glm::vec2& other);

    /** Multiply by 2x2 matrix (vector * matrix). */
    Vector2D operator*(const Matrix<2>& mat) const;

    /** Pointer conversion for APIs requiring float*. */
    operator float*();

    // Accessors / mutators
    float x() const;
    float& x();
    float y() const;
    float& y();

    /** 2D dot product. */
    float dotProduct(const Vector2D& other) const noexcept;
    /** 2D cross product magnitude (scalar z). */
    float crossProductMag(const Vector2D& other) const noexcept;
};

/**
 * @brief 3D vector convenience wrapper with named accessors.
 */
class Vector3D : public Vector<3> {
public:
    Vector3D();
    Vector3D(float x, float y, float z);
    Vector3D(const Vector<3>& other);

    Vector3D& operator=(const glm::vec3& other);
    operator glm::vec3() const;

    Vector3D operator*(const Matrix<3>& mat) const;
    operator float*();

    float x() const;
    float& x();
    float y() const;
    float& y();
    float z() const;
    float& z();

    /** Return magnitude of cross product with other (length of resulting vector). */
    float crossProductMag(const Vector3D& other) const noexcept;
};

/**
 * @brief 4D vector convenience wrapper with named accessors.
 */
class Vector4D : public Vector<4> {
public:
    Vector4D();
    Vector4D(float x, float y, float z, float a);
    Vector4D(const Vector<4>& other);

    Vector4D& operator=(const glm::vec4& other);
    operator glm::vec4() const;

    Vector4D operator*(const Matrix<4>& mat) const;
    operator float*();

    float x() const;
    float& x();
    float y() const;
    float& y();
    float z() const;
    float& z();
    float a() const;
    float& a();
};

//********* Non-member operators / utilities **********//

/**
 * @brief Scalar division (vector / scalar).
 * @return new vector with each component divided by scalar.
 */
template <size_t N>
Vector<N> operator/(const Vector<N>& vec, float scalar) {
    Vector<N> result;
    for (size_t i = 0; i < N; ++i) 
        result[i] = vec[i] / scalar;
    return result;
}

/**
 * @brief Scalar multiplication (vector * scalar).
 */
template <size_t N>
Vector<N> operator*(const Vector<N>& vec, float scalar) {
    Vector<N> result;
    for (size_t i = 0; i < N; ++i) 
        result[i] = vec[i] * scalar;
    return result;
}

/**
 * @brief Stream output for debugging.
 */
template <size_t N>
std::ostream& operator<<(std::ostream& os, const Vector<N>& vector) {
    for (size_t i = 0; i < N; ++i) {
        os << vector[i];
        if (i < N - 1) os << ", ";
    }
    return os;
}

/**
 * @brief Squared distance between two vectors (fast, avoids sqrt).
 */
template <size_t N>
float distanceSquared(const Vector<N>& a, const Vector<N>& b) {
    float result = 0.0f;
    for (size_t i = 0; i < N; ++i) {
        result += pow(b[i] - a[i], 2);
    }
    return result;
}

/**
 * @brief Euclidean distance between vectors.
 */
template <size_t N>
float distance(const Vector<N>& a, const Vector<N>& b) {
    return sqrt(distanceSquared(a, b));
}

/**
 * @brief Dot product helper (non-member).
 */
template <size_t N>
float dotProduct(const Vector<N>& a, const Vector<N>& b) {
    float result = 0.0f;
    for (size_t i = 0; i < N; ++i) {
        result += a[i] * b[i];
    }
    return result;
}

/**
 * @brief Compute length (magnitude) of vector.
 */
template <size_t N>
float length(const Vector<N>& a) {
    float result = 0.0f;
    for (size_t i = 0; i < N; ++i)
        result += a[i] * a[i];
    return sqrt(result);
}

/**
 * @brief Normalize and return vector; logs and returns input unmodified if zero-length.
 */
template <size_t N>
Vector<N> normal(Vector<N>&& a) {
    float result = length(a);
    if (result == 0.0f) {
        logger.error("Cannot normalize a zero-length vector.");
        return a; 
    }

    return a /= result;
}

#endif // VECTOR_H