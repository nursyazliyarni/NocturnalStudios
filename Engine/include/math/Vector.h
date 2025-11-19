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

template <size_t N>
class Vector {
public:
    using value_type = float;
    static constexpr size_t size = N;

    constexpr Vector() noexcept : data{} {}
    constexpr Vector(const Vector& other) noexcept = default;
    constexpr Vector& operator=(const Vector& other) noexcept = default;

    // Variadic ctor for direct component initialization
    template <typename... Args>
        requires (sizeof...(Args) == N && (std::is_convertible_v<Args,float> && ...))
    constexpr explicit Vector(Args... args) noexcept : data{ static_cast<float>(args)... } {}

    // glm interop (keeps existing derived class operators working)
    template <glm::length_t L>
        requires (L == N && (N == 2 || N == 3 || N == 4))
    explicit constexpr Vector(glm::vec<L, float, glm::defaultp> const& v) noexcept {
        for (size_t i = 0; i < N; ++i) data[i] = v[i];
    }

    template <glm::length_t L>
        requires (L == N && (N == 2 || N == 3 || N == 4))
    constexpr operator glm::vec<L, float, glm::defaultp>() const noexcept {
        if constexpr (N == 2) return glm::vec2{ data[0], data[1] };
        else if constexpr (N == 3) return glm::vec3{ data[0], data[1], data[2] };
        else return glm::vec4{ data[0], data[1], data[2], data[3] };
    }

    // Element access
    constexpr float& operator[](size_t i) noexcept { return data[i]; }
    constexpr const float& operator[](size_t i) const noexcept { return data[i]; }

    // Arithmetic
    constexpr Vector& operator+=(const Vector& other) noexcept {
        for (size_t i=0;i<N;++i) data[i] += other.data[i];
        return *this;
    }
    constexpr Vector& operator-=(const Vector& other) noexcept {
        for (size_t i=0;i<N;++i) data[i] -= other.data[i];
        return *this;
    }
    constexpr Vector& operator*=(float s) noexcept {
        for (auto& v : data) v *= s;
        return *this;
    }
    constexpr Vector& operator/=(float s) noexcept {
        // Caller responsibility; existing logger kept in specialized wrapper if needed
        for (auto& v : data) v /= s;
        return *this;
    }
    constexpr Vector operator+(const Vector& other) const noexcept {
        Vector r(*this); r += other; return r;
    }
    constexpr Vector operator-(const Vector& other) const noexcept {
        Vector r(*this); r -= other; return r;
    }
    constexpr Vector operator-() const noexcept {
        Vector r;
        for (size_t i=0;i<N;++i) r.data[i] = -data[i];
        return r;
    }
    constexpr bool operator==(const Vector& other) const noexcept {
        for (size_t i=0;i<N;++i) if (data[i] != other.data[i]) return false;
        return true;
    }

    // Generic dot
    constexpr float dot(const Vector& other) const noexcept {
        float acc = 0.f;
        for (size_t i=0;i<N;++i) acc = std::fma(data[i], other.data[i], acc);
        return acc;
    }

    // Cross only meaningful for N=3; N=2 returns perpendicular scalar magnitude
    constexpr auto cross(const Vector& other) const noexcept {
        if constexpr (N == 3) {
            return Vector<3>{
                data[1] * other.data[2] - data[2] * other.data[1],
                data[2] * other.data[0] - data[0] * other.data[2],
                data[0] * other.data[1] - data[1] * other.data[0]
            };
        } else if constexpr (N == 2) {
            // 2D "cross" magnitude (z-component if vectors embedded in 3D)
            return data[0] * other.data[1] - data[1] * other.data[0];
        } else {
            static_assert(N == 2 || N == 3, "cross only defined for 2D (magnitude) or 3D");
        }
    }

    // Length / normalization
    constexpr float length2() const noexcept { return dot(*this); }
    float length() const noexcept { return std::sqrt(length2()); }
    Vector normalized() const noexcept {
        float len = length();
        return len > 0.f ? (*this) / len : *this;
    }

    friend constexpr Vector operator*(Vector v, float s) noexcept { v *= s; return v; }
    friend constexpr Vector operator*(float s, Vector v) noexcept { v *= s; return v; }
    friend constexpr Vector operator/(Vector v, float s) noexcept { v /= s; return v; }

    float data[N];
};

class Vector2D : public Vector<2> {
public:
    // Constructors
    Vector2D();
    Vector2D(float x, float y);
    Vector2D(const Vector<2>& other);

    // Assignment operators for glm types
    operator glm::vec2() const;
    Vector2D& operator=(const glm::vec2& other);

    // Matrix multiplication
    Vector2D operator*(const Matrix<2>& mat) const;

    // Conversion operator to float pointer
    operator float*();

    // Accessors and Mutators for x
    float x() const;
    float& x();

    // Accessors and Mutators for y
    float y() const;
    float& y();

    float dotProduct(const Vector2D& other) const noexcept;
    float crossProductMag(const Vector2D& other) const noexcept;
};

class Vector3D : public Vector<3> {
public:
    // Constructors
    Vector3D();
    Vector3D(float x, float y, float z);
    Vector3D(const Vector<3>& other);

    // Assignment operators for glm types
    Vector3D& operator=(const glm::vec3& other);
    operator glm::vec3() const;

    // Matrix multiplication
    Vector3D operator*(const Matrix<3>& mat) const;

    // Conversion operator to float pointer
    operator float*();

    // Accessors and Mutators for x
    float x() const;
    float& x();

    // Accessors and Mutators for y
    float y() const;
    float& y();

    // Accessors and Mutators for z
    float z() const;
    float& z();

    float crossProductMag(const Vector3D& other) const noexcept;
};

class Vector4D : public Vector<4> {
public:
    // Constructors
    Vector4D();
    Vector4D(float x, float y, float z, float a);
    Vector4D(const Vector<4>& other);

    // Assignment operators for glm types
    Vector4D& operator=(const glm::vec4& other);
    operator glm::vec4() const;

    // Matrix multiplication
    Vector4D operator*(const Matrix<4>& mat) const;

    // Conversion operator to float pointer
    operator float*();
    
    // Accessors and Mutators for x
    float x() const;
    float& x();

    // Accessors and Mutators for y
    float y() const;
    float& y();

    // Accessors and Mutators for z
    float z() const;
    float& z();

    // Accessors and Mutators for a
    float a() const;
    float& a();
};

//********* Non-member operators **********//

// Scalar division
template <size_t N>
Vector<N> operator/(const Vector<N>& vec, float scalar) {
    Vector<N> result;
    for (size_t i = 0; i < N; ++i) 
        result[i] = vec[i] / scalar;
    return result;
}

// Scalar multiplication
template <size_t N>
Vector<N> operator*(const Vector<N>& vec, float scalar) {
    Vector<N> result;
    for (size_t i = 0; i < N; ++i) 
        result[i] = vec[i] * scalar;
    return result;
}

// Output stream operator
template <size_t N>
std::ostream& operator<<(std::ostream& os, const Vector<N>& vector) {
    for (size_t i = 0; i < N; ++i) {
        os << vector[i];
        if (i < N - 1) os << ", ";
    }
	return os;
}

//********* Useful Functions For Vectors **********//

// Calculate the square distance between two vectors
template <size_t N>
float distanceSquared(const Vector<N>& a, const Vector<N>& b) {
    float result = 0.0f;
    for (size_t i = 0; i < N; ++i) {
        result += pow(b[i] - a[i], 2);
    }
    return result;
}

// Calculate the distance between two vectors
template <size_t N>
float distance(const Vector<N>& a, const Vector<N>& b) {
    return sqrt(distanceSquared(a, b));
}

// Calculate the dot product of two vectors
template <size_t N>
float dotProduct(const Vector<N>& a, const Vector<N>& b) {
    float result = 0.0f;
    for (size_t i = 0; i < N; ++i) {
        result += a[i] * b[i];
    }
    return result;
}

// Calculate the length (magnitude) of a vector
template <size_t N>
float length(const Vector<N>& a) {
    float result = 0.0f;
    for (size_t i = 0; i < N; ++i)
        result += a[i] * a[i];
    return sqrt(result);
}

// Normalize a vector
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