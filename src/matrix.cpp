#include "matrix.h"
#include "exceptions.h"
#include <iostream>
#include <xmmintrin.h>
#include <pmmintrin.h>

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
		1.0f);
	return result;
}

float Vec4::Distance(const Vec4& other) const {
	float dx = x - other.x;
	float dy = y - other.y;
	float dz = z - other.z;

	return std::sqrt(dx * dx + dy * dy + dz * dz);
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
	_nrows = MATRIX_DIMENSIONS;
	_ncols = MATRIX_DIMENSIONS;
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

	//changed internal representation to  data = <row1, row2, row3, row4>
	//but we receive <col1 ... col4>
	for (unsigned i = 0; i < MATRIX_DIMENSIONS; i++) {
		for (unsigned j = 0; j < MATRIX_DIMENSIONS; j++) {
			_data[i + 4 * j] = data[j + 4 * i];
		}
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
	return _data[row * 4 + col];
}

float Matrix::operator() (unsigned row, unsigned col) const {
	if (row >= _nrows || col >= _ncols) {
		throw BadIndexException("Attempting to acess matrix of dimensions"
			+ std::to_string(_nrows) + "x" + std::to_string(_ncols) + " at invalid position ("
			+ std::to_string(row) + "," + std::to_string(col) + ")");
	}
	return _data[row * 4 + col];
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

//note move constructor?
Matrix Matrix::Matmul(const Matrix& left, const Matrix& right) {

	Matrix result;

	__m128 right_row_0 = _mm_loadu_ps(&right.GetData()[0]);
	__m128 right_row_1 = _mm_loadu_ps(&right.GetData()[4]);
	__m128 right_row_2 = _mm_loadu_ps(&right.GetData()[8]);
	__m128 right_row_3 = _mm_loadu_ps(&right.GetData()[12]);

	__m128 new_row_0 = _mm_mul_ps(right_row_0, _mm_set1_ps(left(0, 0)));
	new_row_0 = _mm_add_ps(new_row_0, _mm_mul_ps(right_row_1, _mm_set1_ps(left(0, 1))));
	new_row_0 = _mm_add_ps(new_row_0, _mm_mul_ps(right_row_2, _mm_set1_ps(left(0, 2))));
	new_row_0 = _mm_add_ps(new_row_0, _mm_mul_ps(right_row_3, _mm_set1_ps(left(0, 3))));

	__m128 new_row_1 = _mm_mul_ps(right_row_0, _mm_set1_ps(left(1, 0)));
	new_row_1 = _mm_add_ps(new_row_1, _mm_mul_ps(right_row_1, _mm_set1_ps(left(1, 1))));
	new_row_1 = _mm_add_ps(new_row_1, _mm_mul_ps(right_row_2, _mm_set1_ps(left(1, 2))));
	new_row_1 = _mm_add_ps(new_row_1, _mm_mul_ps(right_row_3, _mm_set1_ps(left(1, 3))));

	__m128 new_row_2 = _mm_mul_ps(right_row_0, _mm_set1_ps(left(2, 0)));
	new_row_2 = _mm_add_ps(new_row_2, _mm_mul_ps(right_row_1, _mm_set1_ps(left(2, 1))));
	new_row_2 = _mm_add_ps(new_row_2, _mm_mul_ps(right_row_2, _mm_set1_ps(left(2, 2))));
	new_row_2 = _mm_add_ps(new_row_2, _mm_mul_ps(right_row_3, _mm_set1_ps(left(2, 3))));

	__m128 new_row_3 = _mm_mul_ps(right_row_0, _mm_set1_ps(left(3, 0)));
	new_row_3 = _mm_add_ps(new_row_3, _mm_mul_ps(right_row_1, _mm_set1_ps(left(3, 1))));
	new_row_3 = _mm_add_ps(new_row_3, _mm_mul_ps(right_row_2, _mm_set1_ps(left(3, 2))));
	new_row_3 = _mm_add_ps(new_row_3, _mm_mul_ps(right_row_3, _mm_set1_ps(left(3, 3))));

	_mm_storeu_ps(&result._data[0], new_row_0);
	_mm_storeu_ps(&result._data[4], new_row_1);
	_mm_storeu_ps(&result._data[8], new_row_2);
	_mm_storeu_ps(&result._data[12], new_row_3);

	return result;
}

Vec4 Matrix::Matmul(const Matrix& mat, const Vec4& vec) {
	Vec4 result{0.0f, 0.0f, 0.0f, 0.0f };
	__m128 mat_row, _vec, res;

	//load vec (in reverse)
	_vec = _mm_set_ps(vec.w, vec.z, vec.y, vec.x);

	//for each row of matrix
	for (int i = 0; i < 4; i++) {
		//load the i-th row of matrix
		mat_row = _mm_loadu_ps(&mat._data[i * 4]);

		//dot product of row and vector
		res = _mm_mul_ps(mat_row, _vec);

		//double horizontal add to get x + y + z + w
		res = _mm_hadd_ps(res, res);
		res = _mm_hadd_ps(res, res);

		_mm_store_ss(&result(i), res);
	}

	return result;
}

Matrix Matrix::Eye(unsigned rows) {
	Matrix mat(rows, rows);
	for (unsigned i = 0; i < rows; i++) {
		mat(i, i) = 1.0f;
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
	mat(3, 3) = 1.0f;

	return mat;
}

Matrix Matrix::Viewport(int x, int y, int width, int height) {
	auto width_half = width / 2;
	auto height_half = height / 2;
	Matrix mat;
	mat(0, 0) = static_cast<float>(width_half);
	mat(1, 1) = static_cast<float>(height_half);
	mat(0, 3) = x + static_cast<float>(width_half);
	mat(1, 3) = y + static_cast<float>(height_half);
	//maps z from [-1,1] to [0,1]
	mat(2, 2) = 1.0f / 2;
	mat(2, 3) = 1.0f / 2;

	mat(3, 3) = 1.0f;

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
		mat(3, i) = 0.0f;
	}
	mat(0, 3) = -eye.dot(xaxis);
	mat(1, 3) = -eye.dot(yaxis);
	mat(2, 3) = -eye.dot(zaxis);
	mat(3, 3) = 1.0f;
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

Matrix Matrix::Frustum3D(float left, float right, float bottom, float top, float near, float far) {
	// Create a perspective projection matrix
	// for derivation see https://www.songho.ca/opengl/gl_projectionmatrix.html

	Matrix mat = Eye(4);
	auto side_difference = right - left;
	auto top_difference = top - bottom;
	auto plane_difference = far - near;
	mat(0, 0) = near * (2 / side_difference);
	mat(0, 2) = (right + left) / side_difference;
	mat(1, 1) =  near * ( 2 / top_difference);
	mat(1, 2) = (top + bottom) / top_difference;
	mat(2, 2) = (-(far + near)) / plane_difference;
	mat(2, 3) = (-2 * far * near) / plane_difference;
	mat(3, 2) = -1;
	mat(3, 3) = 0;
	return mat;

}
