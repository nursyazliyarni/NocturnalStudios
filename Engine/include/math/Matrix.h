// Requires GLM Library
#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <cmath>
#include <array>
#include <iomanip>
#include <type_traits>

#include "../glm/glm/matrix.hpp"
#include "../include/math/Vector.h"

constexpr double PI = 3.14159265358979323846;

using namespace std;

template <size_t N>
class Vector;

//********* Matrix Class Template Declaration **********//

template <size_t N>
class Matrix {
public:
	// Constructors
	Matrix();
	Matrix(std::initializer_list<float> list);
	Matrix(const Matrix& other);

	Matrix(glm::mat3x3 other) requires(N == 3);
	Matrix(glm::mat4x4 other) requires(N == 4);
	operator glm::mat3x3() const;
	operator glm::mat4x4() const;

	// Destructor
	~Matrix() = default;

	// Assignment operator
	Matrix& operator=(const Matrix& other) noexcept;

	// Arithmetic operators
	Matrix operator+(const Matrix& other) const noexcept;
	Matrix operator*(const Matrix& other) const noexcept;
	Vector<N> operator*(const Vector<N>& vec) const noexcept;

	// Assignment operators
	Matrix& operator+=(const Matrix& other) noexcept;
	Matrix& operator-=(const Matrix& other) noexcept;
	Matrix& operator*=(float scalar) noexcept;
	Matrix& operator/=(float scalar);

	// Unary operators
	Matrix operator-() const noexcept;

	Matrix& Transpose() noexcept;
	Matrix& Inverse() noexcept;
	Matrix& Identity() noexcept;

	// Accessors
	float getElement(size_t row, size_t col) const;

	// Row Proxy Class
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

	// Overload operator[] to return a rowProxy
	rowProxy operator[](size_t row) {
		return rowProxy(&data[row * N]);
	}

	const rowProxy operator[](size_t row) const {
		return rowProxy(const_cast<float*>(&data[row * N]));
	}

	// Friend function for output
	friend std::ostream& operator<<(std::ostream& os, const Matrix<N>& mat) {
		for (size_t i{}; i < N; ++i) {
			for (size_t j{}; j < N; ++j) {
				os << std::setw(5) << mat.data[i * N + j] << " ";
			}
			os << std::endl;
		}
		return os;
	}

	// Float conversion operator
	operator float* () {
		return data.data();
	}

public:
	std::array<float, N * N> data;
};

//********* Non-Member Functions **********//

Matrix<3> Translate3x3(const Matrix<3>& transformMatModel, float x, float y);
Matrix<3> RotateRad3x3(const Matrix<3>& transformMatModel, float radAngle);
Matrix<3> RotateDeg3x3(const Matrix<3>& transformMatModel, float degAngle);
Matrix<3> Scale3x3(const Matrix<3>& transformMatModel, float x, float y);

Matrix<4> Translate4x4(const Matrix<4>& transformMatModel, float x, float y, float z);
Matrix<4> RotateRad4x4(const Matrix<4>& transformMatModel, float radAngle);
Matrix<4> RotateDeg4x4(const Matrix<4>& transformMatModel, float degAngle);
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
		// @Todo (axewhy): Handle error to logger
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
	for (size_t i{}; i < N; ++i) {
		result[i] = 0.0f;
		for (size_t j{}; j < N; ++j) {
			result[i] += data[i * N + j] * vec[j];
		}
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
	if (scalar == 0.0f) return *this; // error case - division by zero @Todo (axewhy): Handle error to logger

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
	for (glm::length i = 0; i < N; ++i) {
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
		// @Todo (axewhy): Handle error to logger
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
	for (size_t i{}; i < N ++i) {
		if (i % (N + 1) == 0)
			data[i] = 1.f;
	}
	return *this;
}

// Transpose matrix
template <size_t N>
Matrix<N>& Matrix<N>::Transpose() noexcept {
	Matrix<N> temp = *this;
	for (size_t i{}; i < N; ++i) {
		for (size_t j{}; j < N; ++j) {
			(*this)[i][j] = temp[j][i];
		}
	}
	return *this;
}

// Inverse matrix for 3x3
template <>
inline Matrix<3>& Matrix<3>::Inverse() noexcept {
	float determinant = 0.f;
	Matrix<3> adjugate{};

	// Calculate adjugate matrix
	adjugate.data[0] = data[4] * data[8] - data[5] * data[7];
	adjugate.data[1] = data[2] * data[7] - data[1] * data[8];
	adjugate.data[2] = data[1] * data[5] - data[2] * data[4];

	adjugate.data[3] = data[5] * data[6] - data[3] * data[8];
	adjugate.data[4] = data[0] * data[8] - data[2] * data[6];
	adjugate.data[5] = data[2] * data[3] - data[0] * data[5];

	adjugate.data[6] = data[3] * data[7] - data[4] * data[6];
	adjugate.data[7] = data[1] * data[6] - data[0] * data[7];
	adjugate.data[8] = data[0] * data[4] - data[1] * data[3];

	// Calculate determinant
	determinant = data[0] * adjugate.data[0] + data[1] * adjugate.data[3] + data[2] * adjugate.data[6];

	if (determinant == 0.f) return *this; // error case - non-invertible matrix @Todo (axewhy): Handle error to logger

	// Calculate inverse
	float invDeterminant = 1.f / determinant;
	for (size_t i{}; i < 9; ++i)
		data[i] = adjugate.data[i] * invDeterminant;

	return *this;
}

// Inverse matrix for 4x4
template <>
inline Matrix<4>& Matrix<4>::Inverse() noexcept {
	float determinant = 0.f;
	Matrix<4> adjugate{};

	// Calculate adjugate matrix
    adjugate.data[0] = data[5] * (data[10] * data[15] - data[11] * data[14]) - data[6] * (data[9] * data[15] - data[11] * data[13]) + data[7] * (data[9] * data[14] - data[10] * data[13]);
    adjugate.data[1] = -data[1] * (data[10] * data[15] - data[11] * data[14]) + data[2] * (data[9] * data[15] - data[11] * data[13]) - data[3] * (data[9] * data[14] - data[10] * data[13]);
    adjugate.data[2] = data[1] * (data[6] * data[15] - data[7] * data[14]) - data[2] * (data[5] * data[15] - data[7] * data[13]) + data[3] * (data[5] * data[14] - data[6] * data[13]);
    adjugate.data[3] = -data[1] * (data[6] * data[11] - data[7] * data[10]) + data[2] * (data[5] * data[11] - data[7] * data[9]) - data[3] * (data[5] * data[10] - data[6] * data[9]);

    adjugate.data[4] = -data[4] * (data[10] * data[15] - data[11] * data[14]) + data[6] * (data[8] * data[15] - data[11] * data[12]) - data[7] * (data[8] * data[14] - data[10] * data[12]);
    adjugate.data[5] = data[0] * (data[10] * data[15] - data[11] * data[14]) - data[2] * (data[8] * data[15] - data[11] * data[12]) + data[3] * (data[8] * data[14] - data[10] * data[12]);
    adjugate.data[6] = -data[0] * (data[6] * data[15] - data[7] * data[14]) + data[2] * (data[4] * data[15] - data[7] * data[12]) - data[3] * (data[4] * data[14] - data[6] * data[12]);
    adjugate.data[7] = data[0] * (data[6] * data[11] - data[7] * data[10]) - data[2] * (data[4] * data[11] - data[7] * data[8]) + data[3] * (data[4] * data[10] - data[6] * data[8]);

    adjugate.data[8] = data[4] * (data[9] * data[15] - data[11] * data[13]) - data[5] * (data[8] * data[15] - data[11] * data[12]) + data[7] * (data[8] * data[13] - data[9] * data[12]);
    adjugate.data[9] = -data[0] * (data[9] * data[15] - data[11] * data[13]) + data[1] * (data[8] * data[15] - data[11] * data[12]) - data[3] * (data[8] * data[13] - data[9] * data[12]);
    adjugate.data[10] = data[0] * (data[5] * data[15] - data[7] * data[13]) - data[1] * (data[4] * data[15] - data[7] * data[12]) + data[3] * (data[4] * data[13] - data[5] * data[12]);
    adjugate.data[11] = -data[0] * (data[5] * data[11] - data[7] * data[9]) + data[1] * (data[4] * data[11] - data[7] * data[8]) - data[3] * (data[4] * data[9] - data[5] * data[8]);

    adjugate.data[12] = -data[4] * (data[9] * data[14] - data[10] * data[13]) + data[5] * (data[8] * data[14] - data[10] * data[12]) - data[6] * (data[8] * data[13] - data[9] * data[12]);
    adjugate.data[13] = data[0] * (data[9] * data[14] - data[10] * data[13]) - data[1] * (data[8] * data[14] - data[10] * data[12]) + data[2] * (data[8] * data[13] - data[9] * data[12]);
    adjugate.data[14] = -data[0] * (data[5] * data[14] - data[6] * data[13]) + data[1] * (data[4] * data[14] - data[6] * data[12]) - data[2] * (data[4] * data[13] - data[5] * data[12]);
    adjugate.data[15] = data[0] * (data[5] * data[10] - data[6] * data[9]) - data[1] * (data[4] * data[10] - data[6] * data[8]) + data[2] * (data[4] * data[9] - data[5] * data[8]);

	// Calculate determinant
	determinant = data[0] * adjugate.data[0] + data[1] * adjugate.data[4] + data[2] * adjugate.data[8] + data[3] * adjugate.data[12];

	if (determinant == 0.f) return *this; // error case - non-invertible matrix @Todo (axewhy): Handle error to logger

	// Calculate inverse
	float invDeterminant = 1.f / determinant;
	for (size_t i{}; i < 16; ++i)
		data[i] = adjugate.data[i] * invDeterminant;

	return *this;
}

#endif // MATRIX_H