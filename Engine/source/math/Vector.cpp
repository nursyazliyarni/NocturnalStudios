#include "../include/math/Vector.h"
#include "../include/math/Matrix.h"

//********* 2D Vector Implementation **********//

// Default constructor
Vector2D::Vector2D() : Vector<2>() {}

// Constructor with x and y values
Vector2D::Vector2D(float x, float y) : Vector<2>() {
	data[0] = x;
	data[1] = y;
}

// Copy constructor
Vector2D::Vector2D(const Vector<2>& other) : Vector<2>{ other } {}

// Dot product for 2D vectors
float Vector2D::dotProduct(const Vector2D& vector) const noexcept {
	return data[0] * vector.data[0] + data[1] * vector.data[1];
}

// Cross product magnitude for 2D vectors
float Vector2D::crossProductMag(const Vector2D& vector) const noexcept {
	return data[0] * vector.data[1] - data[1] * vector.data[0];
}

// Assignment operator for glm::vec2
Vector2D::operator glm::vec2() const {
	return { data[0], data[1] };
}

Vector2D& Vector2D::operator=(const glm::vec2& other) {
	this->data[0] = other.x;
	this->data[1] = other.y;
	return *this;
}

// Matrix multiplication
Vector2D Vector2D::operator*(const Matrix<2>& mat) const {
	Vector2D result;
	result.data[0] = mat[0][0] * data[0] + mat[0][1] * data[1];
	result.data[1] = mat[1][0] * data[0] + mat[1][1] * data[1];
	return result;
}

// Conversion operator to float pointer
Vector2D::operator float* () {
	return data;
}

// Accessors
float Vector2D::x() const {
	return data[0];
}

float Vector2D::y() const {
	return data[1];
}

// Mutators
float& Vector2D::x() {
	return data[0];
}

float& Vector2D::y() {
	return data[1];
}

//********* 3D Vector Implementation **********//

// Default constructor
Vector3D::Vector3D() : Vector<3>() {}

// Constructor with x, y and z values
Vector3D::Vector3D(float x, float y, float z) : Vector<3>() {
	data[0] = x;
	data[1] = y;
	data[2] = z;
}

// Copy constructor
Vector3D::Vector3D(const Vector<3>& other) : Vector<3>{ other } {}

// Assignment operator for glm::vec3
Vector3D::operator glm::vec3() const {
	return { data[0], data[1], data[2] };
}

Vector3D& Vector3D::operator=(const glm::vec3& other) {
	this->data[0] = other.x;
	this->data[1] = other.y;
	this->data[2] = other.z;
	return *this;
}

// Matrix multiplication
Vector3D Vector3D::operator*(const Matrix<3>& mat) const {
	Vector3D result;
	result.data[0] = mat[0][0] * data[0] + mat[0][1] * data[1] + mat[0][2] * data[2];
	result.data[1] = mat[1][0] * data[0] + mat[1][1] * data[1] + mat[1][2] * data[2];
	result.data[2] = mat[2][0] * data[0] + mat[2][1] * data[1] + mat[2][2] * data[2];
	return result;
}

// Conversion operator to float pointer
Vector3D::operator float* () {
	return data;
}

// Accessors
float Vector3D::x() const {
	return data[0];
}

float Vector3D::y() const {
	return data[1];
}

float Vector3D::z() const {
	return data[2];
}

// Mutators
float& Vector3D::x() {
	return data[0];
}

float& Vector3D::y() {
	return data[1];
}

float& Vector3D::z() {
	return data[2];
}

// Cross product magnitude for 3D vectors
float Vector3D::crossProductMag(const Vector3D& vec) const noexcept {
	Vector3D temp = {
		data[1] * vec.data[2] - data[2] * vec.data[1],
		data[2] * vec.data[0] - data[0] * vec.data[2],
		data[0] * vec.data[1] - data[1] * vec.data[0]
	};
	return (float)sqrt(pow(temp.data[0], 2) + pow(temp.data[1], 2) + pow(temp.data[2], 2));
}

//********* 4D Vector Implementation **********//

// Default constructor
Vector4D::Vector4D() : Vector<4>() {}

// Constructor with x, y, z and a values
Vector4D::Vector4D(float x, float y, float z, float a) : Vector<4>() {
	data[0] = x;
	data[1] = y;
	data[2] = z;
	data[3] = a;
}

// Copy constructor
Vector4D::Vector4D(const Vector<4>& other) : Vector<4>{ other } {}

// Assignment operator for glm::vec4
Vector4D::operator glm::vec4() const {
	return { data[0], data[1], data[2], data[3] };
}

Vector4D& Vector4D::operator=(const glm::vec4& other) {
	this->data[0] = other.x;
	this->data[1] = other.y;
	this->data[2] = other.z;
	this->data[3] = other.w;
	return *this;
}

// Matrix multiplication
Vector4D Vector4D::operator*(const Matrix<4>& mat) const {
	Vector4D result;
	result.data[0] = mat[0][0] * data[0] + mat[0][1] * data[1] + mat[0][2] * data[2] + mat[0][3] * data[3];
	result.data[1] = mat[1][0] * data[0] + mat[1][1] * data[1] + mat[1][2] * data[2] + mat[1][3] * data[3];
	result.data[2] = mat[2][0] * data[0] + mat[2][1] * data[1] + mat[2][2] * data[2] + mat[2][3] * data[3];
	result.data[3] = mat[3][0] * data[0] + mat[3][1] * data[1] + mat[3][2] * data[2] + mat[3][3] * data[3];
	return result;
}

// Conversion operator to float pointer
Vector4D::operator float* () {
	return data;
}

// Accessors
float Vector4D::x() const {
	return data[0];
}

float Vector4D::y() const {
	return data[1];
}

float Vector4D::z() const {
	return data[2];
}

float Vector4D::a() const {
	return data[3];
}

// Mutators
float& Vector4D::x() {
	return data[0];
}

float& Vector4D::y() {
	return data[1];
}

float& Vector4D::z() {
	return data[2];
}

float& Vector4D::a() {
	return data[3];
}