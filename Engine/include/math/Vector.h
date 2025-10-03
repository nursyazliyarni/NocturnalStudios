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
    // Constructors
    Vector();
    Vector(const Vector& other);
    Vector& operator=(const Vector& other) noexcept;

    // Arithmetic operators
    Vector operator+(const Vector& other) const noexcept;
    Vector operator-(const Vector& other) const noexcept;

    // Assignment operators
    Vector& operator+=(const Vector& other) noexcept; 
    Vector& operator-=(const Vector& other) noexcept;
    Vector& operator*=(float scalar) noexcept;
    Vector& operator/=(float scalar);

    bool operator==(const Vector& other) const noexcept;

    // Unary operators
    Vector operator-() const noexcept;

    // Access operators
    float& operator[](size_t index);
    const float& operator[](size_t index) const;

public:
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

//********* Vector Class Template Implementation **********//

// Default constructor
template <size_t N>
Vector<N>::Vector() {
    for (size_t i = 0; i < N; ++i) 
        data[i] = 0.0f;   
}

// Copy constructor
template <size_t N>
Vector<N>::Vector(const Vector& other) {
    for (size_t i = 0; i < N; ++i)
        data[i] = other.data[i];
}

// Copy assignment operator
template <size_t N>
Vector<N>& Vector<N>::operator=(const Vector& other) noexcept {
    if (this != &other)
        for (size_t i = 0; i < N; ++i)
            data[i] = other.data[i];
    return *this;
}

// Addition operator
template <size_t N>
Vector<N> Vector<N>::operator+(const Vector& other) const noexcept {
    Vector<N> result;
	for (size_t i = 0; i < N; ++i)
        result.data[i] = data[i] + other.data[i];
    return result;
}

// Subtraction operator
template <size_t N>
Vector<N> Vector<N>::operator-(const Vector& other) const noexcept {
    Vector<N> result;
    for (size_t i = 0; i < N; ++i)
        result.data[i] = data[i] - other.data[i];
    return result;
}

// Addition assignment operator
template <size_t N>
Vector<N>& Vector<N>::operator+=(const Vector& other) noexcept {
    for (size_t i = 0; i < N; ++i)
        data[i] += other.data[i];
    return *this;
}

// Subtraction assignment operator
template <size_t N>
Vector<N>& Vector<N>::operator-=(const Vector& other) noexcept {
    for(size_t i = 0; i < N; ++i)
        data[i] -= other.data[i];
    return *this;
}

// Multiplication assignment operator
template <size_t N>
Vector<N>& Vector<N>::operator*=(float scalar) noexcept {
    for (size_t i = 0; i < N; ++i)
        data[i] *= scalar;
    return *this;
}

// Division assignment operator
template <size_t N>
Vector<N>& Vector<N>::operator/=(float scalar) {
    if (scalar == 0.0f) {
        logger.error("Division by zero in vector division assignment.");
		return *this;
    }
    for (size_t i = 0; i < N; ++i)
        data[i] /= scalar;
    return *this;
}

// Equality operator
template <size_t N>
bool Vector<N>::operator==(const Vector<N>& other) const noexcept {
    for (size_t i = 0; i < N; ++i)
        if (this->data[i] != other.data[i]) return false;
	return true;
}

// Unary negation operator
template <size_t N>
Vector<N> Vector<N>::operator-() const noexcept {
    Vector<N> result;
    for (size_t i = 0; i < N; ++i)
        result[i] = -data[i];
    return result;
}

// Access operator (non-const)
template <size_t N>
float& Vector<N>::operator[](size_t index) {
    return data[index];
}

// Access operator (const)
template <size_t N>
const float& Vector<N>::operator[](size_t index) const {
    return data[index];
}

#endif // VECTOR_H