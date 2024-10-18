#include "matrix.h"
#include "exceptions.h"
#include <iostream>


void Vec4::PerspectiveDivide() {
	if (w == 0) {
		return;
	}
	x = x / w;
	y = y / w;
	z = z / w;
	w = 1;
}

float Vec4::dot(const Vec4& vec) const {
	return x * vec.x + y * vec.y + z * vec.z;
}

void Vec4::normalize() {
	float norm = sqrt(x * x + y * y + z * z);
	x = x / norm;
	y = y / norm;
	z = z / norm;
}

Vec4 Vec4::operator* (float scalar) {
	return Vec4{x*scalar, y*scalar, z*scalar, w*scalar};
}

Vec4& Vec4::operator*= (float scalar) {
	x *= scalar;
	y *= scalar;
	z *= scalar;
	w *= scalar;
	return *this;
}

Vec4 Vec4::Cross3D(const Vec4& vec1, const Vec4& vec2) {
	Vec4 result = Vec4(
		vec1.y * vec2.z - (vec1.z * vec2.y),
		vec1.z * vec2.x - (vec1.x * vec2.z),
		vec1.x * vec2.y - (vec1.y * vec2.x),
		1);
	return result;
}

Vec4 operator+ (Vec4 vec1, const Vec4& vec2) {
	vec1 += vec2;
	return vec1;
}

Vec4& Vec4::operator+= (const Vec4& other) {
	x += other.x;
	y += other.y;
	z += other.z;
	w += other.w;
	return *this;
}

Vec4 operator- (Vec4 vec1, const Vec4& vec2) {
	vec1 -= vec2;
	return vec1;
}

Vec4& Vec4::operator-= (const Vec4& other) {
	x -= other.x;
	y -= other.y;
	z -= other.z;
	w -= other.w;
	return *this;
}


void Vec4::PrintVector(const Vec4& vec) {
	for (unsigned i = 0; i < 4; i++) {
		std::cout << vec(i) << " ";
	}
	std::cout << std::endl;
	std::cout << "Finished printing vector" << std::endl;
}


float& Vec4::operator() (unsigned position) {
	switch (position) {
	case sglAxis::X_AXIS:
		return x;
	case sglAxis::Y_AXIS:
		return y;
	case sglAxis::Z_AXIS:
		return z;
	case sglAxis::W_AXIS:
		return w;
	default:
		throw BadIndexException("Attempting to access vector at invalid position " + std::to_string(position));
	}
}

float Vec4::operator() (unsigned position) const {
	switch (position) {
	case sglAxis::X_AXIS:
		return x;
	case sglAxis::Y_AXIS:
		return y;
	case sglAxis::Z_AXIS:
		return z;
	case sglAxis::W_AXIS:
		return w;
	default:
		throw BadIndexException("Attempting to access vector at invalid position " + std::to_string(position));
	}
}

Matrix::Matrix() {
	_nrows = 4;
	_ncols = 4;
	_data.fill(0.0f);
}

Matrix::~Matrix() {}

Matrix::Matrix(unsigned rows, unsigned cols) {
	if (rows < 1 || cols < 1) {
		throw BadDimensionException("Attempting to create matrix with invalid dimensions " 
		+ std::to_string(rows) + "x" + std::to_string(cols));
	}
	_nrows = rows;
	_ncols = cols;
	_data.fill(0.0f);
}

Matrix::Matrix(unsigned rows, unsigned cols, const float* data) {
	if (rows < 1 || cols < 1) {
		throw BadDimensionException("Attempting to create matrix with invalid dimensions "
			+ std::to_string(rows) + "x" + std::to_string(cols));
	}
	_nrows = rows;
	_ncols = cols;
	for (unsigned i = 0; i < rows * cols; i++) {
		_data[i] = data[i];
	}
}

Matrix::Matrix(unsigned rows, unsigned cols, const std::array<float, 16>& data) {
	if (rows < 1 || cols < 1) {
		throw BadDimensionException("Attempting to create matrix with invalid dimensions "
			+ std::to_string(rows) + "x" + std::to_string(cols));
	}
	_nrows = rows;
	_ncols = cols;
	for (unsigned i = 0; i < rows * cols; i++) {
		_data[i] = data[i];
	}
}

float& Matrix::operator() (unsigned row, unsigned col) {
	if (row >= _nrows || col >= _ncols) {
		throw BadIndexException("Attempting to acess matrix of dimensions"
			+ std::to_string(_nrows) + "x" + std::to_string(_ncols) + " at invalid position ("
			+ std::to_string(row) + "," + std::to_string(col) + ")");
	}
	return _data[col * _nrows + row];
}

float Matrix::operator() (unsigned row, unsigned col) const {
	if (row >= _nrows || col >= _ncols) {
		throw BadIndexException("Attempting to acess matrix of dimensions"
			+ std::to_string(_nrows) + "x" + std::to_string(_ncols) + " at invalid position ("
			+ std::to_string(row) + "," + std::to_string(col) + ")");
	}
	return _data[col * _nrows + row];
}

std::pair<unsigned, unsigned> Matrix::GetDimensions() const {
	return std::make_pair(_nrows, _ncols);
}

const std::array<float, 16>& Matrix::GetData() const {
	return _data;
}

void Matrix::PrintMatrix(const Matrix& matrix) {
	auto dims = matrix.GetDimensions();
	std::cout << "Matrix dims " << dims.first << "x" << dims.second << std::endl;
	for (unsigned i = 0; i < dims.second; i++) {
		for (unsigned j = 0; j < dims.first; j++) {
			std::cout << matrix(j, i) << " ";
		}
		std::cout << std::endl;
	}
	std::cout << "Finished printing matrix " << std::endl;
}

/// <summary>
/// Implemented naively for now. For speed will need to optimize this
/// </summary>
Matrix Matrix::Matmul(const Matrix& left, const Matrix& right) {
	auto left_dimensions = left.GetDimensions();
	auto right_dimensions = right.GetDimensions();
	unsigned left_rows, left_cols, right_rows, right_cols;
	left_rows = left_dimensions.first;
	left_cols = left_dimensions.second;
	right_rows = right_dimensions.first;
	right_cols = right_dimensions.second;


	if (left_cols != right_rows) {
		throw BadDimensionException("Attempting to multiply matrices with incompatible dimensions"
			+ std::to_string(left_rows) + "x" + std::to_string(left_cols) + " and "
			+ std::to_string(right_rows) + "x" + std::to_string(right_cols));
	}

	Matrix result;

	for (unsigned i = 0; i < right_cols; ++i) {
		for (unsigned j = 0; j < left_rows; ++j) {
			float sum = 0;
			for (unsigned k = 0; k < left_cols; ++k) {
				sum += left(j, k) * right(k, i);
			}
			result(j, i) = sum;
		}
	}

	return result;
}

Vec4 Matrix::Matmul(Matrix& mat, const Vec4& vec) {
	// also works with 3x3 and 2x2 matrices,
	// in which case the appropriate components of the output
	// will be zeroed.

	/*std::cout << "Multiplying matrix" << std::endl;
	Matrix::PrintMatrix(mat);
	std::cout << "with vector " << std::endl;
	Vec4::PrintVector(vec);*/
	auto mat_dims = mat.GetDimensions();
	unsigned n_rows, n_cols;
	n_rows = mat_dims.first;
	n_cols = mat_dims.second;

	if (n_cols > 4) {
		throw BadDimensionException("Attempting to multiply matrix with "
			+ std::to_string(n_cols) + " columns and 4 component vector");
	}

	float vec_data[4] = {};
	for (unsigned j = 0; j < n_rows; j++) {
		float sum = 0;
		for (unsigned k = 0; k < n_cols; k++) {
			sum += mat(j, k) * vec(k); //TODO check
			//std::cout << "Adding " << mat(j, k) * vec(k) << " to vector component " << j << std::endl;
			//sum += _data[j + k * n_rows] * vec(k);
		}
		vec_data[j] = sum;
	}
	auto ret = Vec4(vec_data[0], vec_data[1], vec_data[2], vec_data[3]);
	//Vec4::PrintVector(ret);
	return ret;
}

Matrix Matrix::Eye(unsigned rows) {
	Matrix mat(rows, rows);
	for (unsigned i = 0; i < rows; i++) {
		mat(i, i) = 1;
	}
	return mat;
}

Matrix Matrix::Scale(float scaleX, float scaleY, float scaleZ) {
	Matrix mat;
	mat(0, 0) = scaleX;
	mat(1, 1) = scaleY;
	mat(2, 2) = scaleZ;
	mat(3, 3) = 1.0f;
	return mat;
}

Matrix Matrix::Rotation3D(float angle, sglAxis axis) {
	//C++ sin and cos need input in radians so convert angle first
	//float rads = angle * PI / 180;
	float cosangle = std::cos(angle);
	float sinangle = std::sin(angle);
	Matrix mat;
	for (unsigned i = 0; i < 4; i++) {
		mat(i, i) = (i == axis || i == 3) ? 1.0f : cosangle;
	}
	switch (axis) {
	case sglAxis::X_AXIS:
		mat(2, 1) = sinangle;
		mat(1, 2) = -sinangle;
		break;
	case sglAxis::Y_AXIS:
		mat(2, 0) = sinangle;
		mat(0, 2) = -sinangle;
		break;
	case sglAxis::Z_AXIS:
		mat(1, 0) = sinangle;
		mat(0, 1) = -sinangle;
		break;
	default:
		throw BadDimensionException("Trying to create rotation matrix alongside invalid axis "
			+ std::to_string(axis));
	}
	return mat;
}


Matrix Matrix::Translation3D(float x, float y, float z) {
	Matrix mat = Matrix::Eye(4);
	mat(0, 3) = x;
	mat(1, 3) = y;
	mat(2, 3) = z;
	return mat;
}

Matrix Matrix::Orthographic3D(float left, float right, float top
	, float bottom, float near, float far) {

	Matrix mat;
	auto side_difference = right - left;
	auto top_difference = top - bottom;
	auto plane_difference = far - near;
	mat(0, 0) = 2 / side_difference;
	mat(1, 1) = 2 / top_difference;
	mat(2, 2) = -2 / plane_difference;
	mat(0, 3) = -(right + left) / side_difference;
	mat(1, 3) = -(top + bottom) / top_difference;
	mat(2, 3) = -(far + near) / plane_difference;
	mat(3, 3) = 1;

	return mat;
}

Matrix Matrix::Viewport(int x, int y, int width, int height) {
	auto width_half = width / 2;
	auto height_half = height / 2;
	Matrix mat;
	mat(0, 0) = width_half;
	mat(1, 1) = height_half;
	mat(0, 3) = x + width_half;
	mat(1, 3) = y + height_half;
	mat(2, 2) = 1;
	mat(3, 3) = 1;

	return mat;
}

Matrix Matrix::LookAt(const Vec4& eye, const Vec4& center, const Vec4& up) {
	Matrix mat;
	Vec4 zaxis = eye - center;
	zaxis.normalize();
	Vec4 xaxis = Vec4::Cross3D(up, zaxis);
	xaxis.normalize();
	Vec4 yaxis = Vec4::Cross3D(zaxis, xaxis);

	for (int i = 0; i < 3; i++) {
		mat(0, i) = xaxis(i);
		mat(1, i) = yaxis(i);
		mat(2, i) = zaxis(i);
		mat(3, i) = 0;
	}
	mat(0, 3) = -eye.dot(xaxis);
	mat(1, 3) = -eye.dot(yaxis);
	mat(2, 3) = -eye.dot(zaxis);
	mat(3, 3) = 1;
	return mat;
}

Matrix Matrix::RotateAroundCenter(float x, float y, float angle) {
	//to perform rotation with a point given as center first translate to point to 
	// be at the origin of coordinate system, rotate and then translate back
	// however, because of matrix transformations being applied from right
	// to left we actually need to do this in reverse order
	Matrix translate_back = Translation3D(x, y, 0);
	Matrix rotate = Rotation3D(angle, sglAxis::Z_AXIS);
	Matrix to_origin = Translation3D(-x, -y, 0);
	Matrix mat = Matmul(translate_back, Matmul(rotate, to_origin));

	return mat;
}
