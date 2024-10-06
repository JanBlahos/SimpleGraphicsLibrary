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
float Vec4::dot(std::shared_ptr<Vec4> vec) {
	return x * vec->x + y * vec->y + z * vec->z;
}

void Vec4::normalize() {
	float norm = sqrt(x * x + y * y + z * z);
	x = x / norm;
	y = y / norm;
	z = z / norm;

}
void Vec4::operator* (float scalar) {
	x *= scalar;
	y *= scalar;
	z *= scalar;
}

std::shared_ptr<Vec4> Vec4::Cross3D(std::shared_ptr<Vec4> vec1, std::shared_ptr<Vec4> vec2) {
	return std::make_shared<Vec4>(vec1->y * vec2->z - (vec1->z * vec2->y),
		vec1->z * vec2->x - (vec1->x * vec2->z),
		vec1->x * vec2->y - (vec1->y * vec2->x), 1);
}

std::shared_ptr<Vec4> operator+ (std::shared_ptr<Vec4> vec1, std::shared_ptr<Vec4> vec2) {
	return std::make_shared<Vec4>(vec1->x + vec2->x, vec1->y + vec2->y, vec1->z + vec2->z, 1);
}

std::shared_ptr<Vec4> operator- (std::shared_ptr<Vec4> vec1, std::shared_ptr<Vec4> vec2) {
	return std::make_shared<Vec4>(vec2->x - vec1->x, vec2->y - vec1->y, vec2->z - vec1->z, 1);
}


float& Vec4::operator() (unsigned position) {
	switch (position) {
	case sglAxis::X_AXIS:
		return x;
		break;
	case sglAxis::Y_AXIS:
		return y;
		break;
	case sglAxis::Z_AXIS:
		return z;
		break;
	case sglAxis::W_AXIS:
		return w;
		break;
	default:
		throw BadIndexException("Attempting to access vector at invalid position " + std::to_string(position));
	}
}

float Vec4::operator() (unsigned position) const {
	switch (position) {
	case sglAxis::X_AXIS:
		return x;
		break;
	case sglAxis::Y_AXIS:
		return y;
		break;
	case sglAxis::Z_AXIS:
		return z;
		break;
	case sglAxis::W_AXIS:
		return w;
		break;
	default:
		throw BadIndexException("Attempting to access vector at invalid position " + std::to_string(position));
	}
}

Matrix::Matrix(unsigned rows, unsigned cols) {
	if (rows < 1 || cols < 1) {
		throw BadDimensionException("Attempting to create matrix with invalid dimensions " 
		+ std::to_string(rows) + "x" + std::to_string(cols));
	}
	_nrows = rows;
	_ncols = cols;
	try {
		_data = new float[rows * cols]();
	}
	catch (std::bad_alloc& ex) {
		throw OutOfMemoryException("Out of memory! Failed to allocate matrix of size "
			+ std::to_string(_nrows) + "X" + std::to_string(_ncols));
	}

}
Matrix::Matrix(unsigned rows, unsigned cols, const float* data) {
	if (rows < 1 || cols < 1) {
		throw BadDimensionException("Attempting to create matrix with invalid dimensions "
			+ std::to_string(rows) + "x" + std::to_string(cols));
	}
	_nrows = rows;
	_ncols = cols;
	try {
		_data = new float[rows * cols]();
	}
	catch (std::bad_alloc& ex) {
		throw OutOfMemoryException("Out of memory! Failed to allocate matrix of size "
			+ std::to_string(_nrows) + "X" + std::to_string(_ncols));
	}
	for (int i = 0; i < rows * cols; i++) {
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

std::pair<unsigned, unsigned> Matrix::GetDimensions() {
	return std::make_pair(_nrows, _ncols);
}

const float* Matrix::GetData() {
	return _data;
}

/// <summary>
/// Implemented naively for now. For speed will need to optimize this
/// </summary>
void Matrix::Matmul(std::shared_ptr<Matrix> multiplicand) {
	auto other_dimensions = multiplicand->GetDimensions();
	auto other_cols = other_dimensions.second;
	if (_ncols != other_dimensions.first) {
		throw BadDimensionException("Attempting to multiply matrices with incompatible dimensions"
			+ std::to_string(_nrows) + "x" + std::to_string(_ncols) + " and "
			+ std::to_string(other_dimensions.first) + "x" + std::to_string(other_cols));
	}
	auto old_data = _data;
	try {
		_data = new float[_nrows * other_cols]();
	}
	catch (std::bad_alloc& ex) {
		throw OutOfMemoryException("Out of memory! Failed to allocate matrix of size "
			+ std::to_string(_nrows) + "X" + std::to_string(other_cols));
	}
	for (unsigned i = 0; i < other_cols; i++) {
		for (unsigned j = 0; j < _nrows; j++) {
			unsigned sum = 0;
				for (unsigned k = 0; k < _ncols; k++) {
					sum += old_data[j + k * _nrows] * (*multiplicand)(k, i);
					}
			_data[j + i * _nrows] = sum;
		}
	}
	delete[] old_data;
}
std::shared_ptr<Vec4> Matrix::Matmul(std::shared_ptr<Vec4> multiplicand) {
	// also works with 3x3 and 2x2 matrices,
	// in which case the appropriate components of the output
	// will be zeroed.
	if (_ncols > 4) {
		throw BadDimensionException("Attempting to multiply matrix with "
			+ std::to_string(_ncols) + " columns and 4 component vector");
	}
	//auto other_cols  = 1;
	float vec_data[4] = {};
	for (unsigned j = 0; j < _nrows; j++) {
		unsigned sum = 0;
		for (unsigned k = 0; k < _ncols; k++) {
			sum += _data[j + k * _nrows] * (*multiplicand)(k);
		}
		vec_data[j] = sum;
	}
	_nrows = 0;
	_ncols = 0;
	delete[] _data;
	return std::make_shared<Vec4>(Vec4(vec_data[0], vec_data[1], vec_data[2], vec_data[3]));
}

Matrix::~Matrix() {
	_nrows = 0;
	_ncols = 0;
	delete[] _data;
}

std::shared_ptr<Matrix> Matrix::Eye(unsigned rows) {
	auto matrix = std::make_shared<Matrix>(Matrix(rows, rows));
	for (unsigned i = 0; i < rows; i++) {
		(*matrix)(i, i) = 1;
	}
	return matrix;
}

std::shared_ptr<Matrix> Matrix::Scale(unsigned rows, float scaleX, float scaleY, float scaleZ) {
	// create a scale matrix one dimension higher than supplied so 
	// it can be multiplied with affine transformations
	auto matrix = std::make_shared<Matrix>(Matrix(rows + 1, rows + 1));;
	(*matrix)(0, 0) = scaleX;
	(*matrix)(1, 1) = scaleY;
	(*matrix)(2, 2) = scaleZ;
	(*matrix)(rows, rows) = 1.0f;
	return matrix;
}
std::shared_ptr<Matrix> Matrix::Rotation3D(float angle, sglAxis axis) {
	//C++ sin and cos need input in radians so convert angle first
	float rads = angle * PI / 180;
	float cosangle = std::cos(rads);
	float sinangle = std::sin(rads);
	auto matrix = std::make_shared<Matrix>(Matrix(4, 4));;
	for (unsigned i = 0; i < 4; i++) {
		(*matrix)(i, i) = (i == axis || i == 3) ? 1.0f : cosangle;
	}
	switch (axis) {
	case sglAxis::X_AXIS:
		(*matrix)(2, 1) = sinangle;
		(*matrix)(1, 2) = -sinangle;
		break;
	case sglAxis::Y_AXIS:
		(*matrix)(2, 0) = sinangle;
		(*matrix)(0, 2) = -sinangle;
		break;
	case sglAxis::Z_AXIS:
		(*matrix)(1, 0) = sinangle;
		(*matrix)(0, 1) = -sinangle;
		break;
	default:
		throw BadDimensionException("Trying to create rotation matrix alongside invalid axis "
			+ std::to_string(axis));
	}
	return matrix;
}


std::shared_ptr<Matrix> Matrix::Translation3D(float x, float y, float z) {
	auto matrix = std::make_shared<Matrix>(Matrix(4, 4));
	(*matrix)(0, 3) = x;
	(*matrix)(1, 3) = y;
	(*matrix)(2, 3) = z;
	(*matrix)(3, 3) = 1.0f;
	return matrix;
}

std::shared_ptr<Matrix> Matrix::Orthographic3D(float left, float right, float top
	, float bottom, float near, float far) {
	auto matrix = std::make_shared<Matrix>(Matrix(4, 4));
	//multiply the far plane by -1
	far = -far;
	auto side_difference = right - left;
	auto top_difference = top - bottom;
	auto plane_difference = near - far;
	(*matrix)(0, 0) = 2 / side_difference;
	(*matrix)(1, 1) = 2 / top_difference;
	(*matrix)(2, 2) = -2 / plane_difference;
	(*matrix)(0, 3) = -right - left / side_difference;
	(*matrix)(1, 3) = -top - bottom / top_difference;
	(*matrix)(2, 3) = -near - far / plane_difference;
	(*matrix)(3, 3) = 1;
	return matrix;
}

std::shared_ptr<Matrix> Matrix::Viewport(int x, int y,
	int width, int height) {
	auto width_half = width / 2;
	auto height_half = height / 2;
	auto matrix = std::make_shared<Matrix>(Matrix(3, 3));
	(*matrix)(0, 0) = width_half;
	(*matrix)(1, 1) = height_half;
	(*matrix)(0, 2) = x + width_half;
	(*matrix)(1, 2) = y + height_half;
	(*matrix)(2, 2) = 1;
	return matrix;
}

std::shared_ptr<Matrix> Matrix::LookAt(std::shared_ptr<Vec4> eye, std::shared_ptr<Vec4> center,
	std::shared_ptr<Vec4> up) {
	auto matrix = std::make_shared<Matrix>(Matrix(4, 4));
	auto zaxis = eye - center;
	zaxis->normalize();
	auto xaxis = Vec4::Cross3D(up, zaxis);
	xaxis->normalize();
	auto yaxis = Vec4::Cross3D(zaxis, xaxis);
	for (int i = 0; i < 3; i++) {
		(*matrix)(0, i) = (*xaxis)(i);
		(*matrix)(1, i) = (*yaxis)(i);
		(*matrix)(2, i) = (*zaxis)(i);
		(*matrix)(3, i) = 0;
	}
	(*matrix)(0, 3) = -eye->dot(xaxis);
	(*matrix)(1, 3) = -eye->dot(yaxis);
	(*matrix)(2, 3) = -eye->dot(zaxis);
	(*matrix)(3, 3) = 1;
	return matrix;
}

