// Requires GLM Library
#include "../include/math/Matrix.h"

//********* 3x3 Non-Member Functions Implementation  **********//
Matrix<3> Translate3x3(const Matrix<3>& transformMatModel, float x, float y) {
	Matrix<3> translationMatrix;
	translationMatrix.Identity();

	translationMatrix.data[2] = x;
	translationMatrix.data[5] = y;

	return transformMatModel * translationMatrix;
}

Matrix<3> RotateRad3x3(const Matrix<3>& transformMatModel, float radAngle) {
	Matrix<3> rotationMatrix;
	rotationMatrix.Identity();

	rotationMatrix.data[0] = cos(radAngle);
	rotationMatrix.data[1] = -sin(radAngle);
	rotationMatrix.data[3] = sin(radAngle);
	rotationMatrix.data[4] = cos(radAngle);

	return transformMatModel * rotationMatrix;
}

Matrix<3> RotateDeg3x3(const Matrix<3>& transformMatModel, float degAngle) {
	float radAngle = degAngle * (static_cast<float>(PI) / 180.f);
	return RotateRad3x3(transformMatModel, radAngle);
}

Matrix<3> Scale3x3(const Matrix<3>& transformMatModel, float x, float y) {
	Matrix<3> scaleMatrix;
	scaleMatrix.Identity();

	scaleMatrix.data[0] = x;
	scaleMatrix.data[4] = y;

	return transformMatModel * scaleMatrix;
}

//********* 4x4 Non-Member Functions Implementation  **********//
Matrix<4> Translate4x4(const Matrix<4>& transformMatModel, float x, float y, float z) {
	Matrix<4> translationMatrix;
	translationMatrix.Identity();
	translationMatrix.data[3] = x;
	translationMatrix.data[7] = y;
	translationMatrix.data[11] = z;

	return transformMatModel * translationMatrix;
}

Matrix<4> RotateRad4x4(const Matrix<4>& transformMatModel, float radAngle) {
	Matrix<4> rotationMatrix;
	rotationMatrix.Identity();

	rotationMatrix.data[0] = cos(radAngle);
	rotationMatrix.data[1] = -sin(radAngle);
	rotationMatrix.data[4] = sin(radAngle);
	rotationMatrix.data[5] = cos(radAngle);

	return transformMatModel * rotationMatrix;
}

Matrix<4> RotateDeg4x4(const Matrix<4>& transformMatModel, float degAngle) {
	float radAngle = degAngle * (static_cast<float>(PI) / 180.f);
	return RotateRad4x4(transformMatModel, radAngle);
}

Matrix<4> Scale4x4(const Matrix<4>& transformMatModel, float x, float y, float z) {
	Matrix<4> scaleMatrix;
	scaleMatrix.Identity();

	scaleMatrix.data[0] = x;
	scaleMatrix.data[5] = y;
	scaleMatrix.data[10] = z;

	return transformMatModel * scaleMatrix;
}
