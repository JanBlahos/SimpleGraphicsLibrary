//---------------------------------------------------------------------------
// matrix.h
// header file containing wrapper classes for matrix representation
// and operations such as multiplication or creating transformation matrices
//---------------------------------------------------------------------------

#ifndef _MATRIX_H_
#define _MATRIX_H_
#include <utility>
#include <cmath>
#include <memory>

//Would need to include math.h for PI constant
constexpr float PI = 3.14159265358979323846;


typedef enum {
	X_AXIS = 0,
	Y_AXIS,
	Z_AXIS,
	W_AXIS
}sglAxis;

/// <summary>
/// Class representing a 4 dimensional vector with xyzw components
/// </summary>
class Vec4 {
public:
	float x;
	float y;
	float z;
	float w;
	/// <summary>
	/// Costruct a 4 component vector with w component.
	/// </summary>
	/// <param name="x"> x axis</param>
	/// <param name="y"> y axis</param>
	/// <param name="z"> z axis </param>
	/// <param name="w"> w component. By default 1 for points and 0 for direction vectors.</param>
	Vec4(float x, float y, float z, float w = 1) : x(x), y(y), z(z), w(w){

	}

	/// <summary>
	/// Subscript operators directly to the vector.
	/// </summary>
	float& operator() (unsigned position);
	float operator() (unsigned position) const;

	/// <summary>
	/// Divide the vector by w so that x = x/w, y = y/w, z= z/w, w = 1.
	/// Does not change direction vectors with w = 0.
	/// </summary>
	void PerspectiveDivide();

	/// <summary>
	/// Normalize the vector components, turning it into a unit vector.
	/// </summary>
	void normalize();

	/// <summary>
	/// Compute the dot product between the current and the given vector
	/// ignoring the w component.
	/// </summary>
	/// <param name="vec"></param>
	/// <returns></returns>
	float dot(std::shared_ptr<Vec4> vec);
	
	/// <summary>
	/// Multiply the vector elemets by a scalar. Only the xyz components
	/// and the w component is ignored.
	/// </summary>
	/// <param name="scalar"></param>
	std::shared_ptr<Vec4> operator* (float scalar);

	/// <summary>
	/// Debugging metod to print given vector.
	/// </summary>
	static void PrintVector(std::shared_ptr<Vec4> vec);

	/// <summary>
	/// Compute the vector product (eg. a vector perpendicular
	/// to both given vectors) betwen the two vectors. Here these
	/// are treated as 3D vectors and w component is disregarded (so 
	/// perspective division should be performed first).
	/// </summary>
	/// <param name="vec1"></param>
	/// <param name="vec2"></param>
	/// <returns></returns>
	static std::shared_ptr<Vec4> Cross3D(std::shared_ptr<Vec4> vec1, std::shared_ptr<Vec4> vec2);
	/// <summary>
	/// Operator adding two vector together.
	/// </summary>
	/// <param name="vec1"></param>
	/// <param name="vec2"></param>
	/// <returns></returns>
	std::shared_ptr<Vec4> operator+ (std::shared_ptr<Vec4> vec2);

	/// <summary>
	/// Operator subtracting two vecotrs (result can be
	/// interpreted as vector representing path from vec2 
	/// to vec1)
	/// <param name="vec1"></param>
	/// <param name="vec2"></param>
	/// <returns></returns>
	std::shared_ptr<Vec4> operator- (std::shared_ptr<Vec4> vec2);
};


/// <summary>
/// Class represanting general matrix of shape rows x cols.
/// By default initialized to zeros.
/// </summary>
class Matrix {
public: 
	/// <summary>
	///  Instantiate a matrix of shape rows x cols.
	/// The matrix data is stored in a column major order.
	/// </summary>
	/// <param name="rows"> Number of rows of the matrix</param>
	/// <param name="cols"> Number of columns of the matrix</param>
	Matrix(unsigned rows, unsigned cols);
	/// <summary>
	///  Instantiate a matrix of shape rows x cols and fill it with given data.
	/// The matrix data is stored in a column major order.
	/// </summary>
	/// <param name="rows"> Number of rows of the matrix</param>
	/// <param name="cols"> Number of columns of the matrix</param>
	/// <param name = "data"> Data of the matrix </param>
	Matrix(unsigned rows, unsigned cols, const float* data);

	~Matrix();
	
	/// <summary>
	/// Subscript operators directly to the matrix.
	/// </summary>
	float& operator() (unsigned row, unsigned col);
	float operator() (unsigned row, unsigned col) const;


	/// <summary>
	/// Get dimensions of the matrix in rows x columns order
	/// </summary>
	/// <returns> Pair when first number is number of rows and second number of columns</returns>
	std::pair<unsigned, unsigned> GetDimensions();

	/// <summary>
	/// Return the matrix data in a float array
	/// </summary>
	/// <returns></returns>
	const float* GetData();

	/// <summary>
	///  Multiply current matrix with the given
	/// matrix from right and return the result.
	/// </summary>
	/// <param name="multiplicand"> The matrix stored in column major order
	/// to multiply current matrix with from the right</param>
	void Matmul(std::shared_ptr<Matrix> multiplicand);

	/// <summary>
	/// An overload of matrix multiplication for a multiplication with vector.
	/// This function also works with 3 x 3 or 2 x 2 matrices,
	/// in which case the appropriate components of the output vector will be zeroed.
	/// </summary>
	/// <param name="multiplicand"></param>
	std::shared_ptr<Vec4> Matmul(std::shared_ptr<Vec4> multiplicand);



	/// <summary>
	///  Returns an identity matrix of dimensionality rows x rows
	/// </summary>
	/// <param name="rows">Dimensions of the matrix</param>
	/// <returns>An identity matrix</returns>
	static std::shared_ptr<Matrix> Eye(unsigned rows);


	/// <summary>
	/// Basic matrix transformations
	/// </summary>

	/// <summary>
	/// Create a scale matrix of given dimension. The actual matrix
	/// will be one dimension higher of rows + 1 x rows + 1
	/// </summary>
	/// <param name="rows">Dimension of the matrix (eg. 3 for
	/// 3D matrix) </param>
	/// <param name="scaleX"> Scale scalar for x axis </param>
	/// <param name="scaleY"> Scale scalar for y axis </param>
	/// <param name="scaleZ"> Scale scalar for z axis </param>
	/// <returns></returns>
	static std::shared_ptr<Matrix> Scale(unsigned rows, float scaleX, float scaleY, float scaleZ);
	/// <summary>
	/// Create a 4x4 rotation matrix around a given axis
	/// </summary>
	/// <param name="angle">Rotation angle in degrees</param>
	/// <param name="axis">Axis around which to rotate</param>
	/// <returns></returns>
	static std::shared_ptr<Matrix> Rotation3D(float angle, sglAxis axis);
	/// <summary>
	/// Create 4x4 translation matrix, with the translation amount
	/// by the individual parameters.
	/// </summary>
	/// <param name="x"> Translation in x axis </param> 
	/// <param name="y"> Translation in y axis </param> 
	/// <param name="z"> Translation in z axis </param> 
	/// <returns></returns>
	static std::shared_ptr<Matrix> Translation3D(float x, float y, float z);

	/// <summary>
	/// Create and orthographic projection matrix,
	/// with the given cube parameters and far and near plane.
	/// </summary>
	/// <param name="left"> Left side of the cube</param>
	/// <param name="right"> Right side of the cube</param>
	/// <param name="top"> Top side of the cube</param>
	/// <param name="bottom"> Bottom side of the cube</param>
	/// <param name="near"> Location of the near plane</param>
	/// <param name="far"> Location of the far plane.</param>
	/// <returns></returns>
	static std::shared_ptr<Matrix> Orthographic3D(float left, float right, float top
		, float bottom, float near, float far);

	/// <summary>
	/// Create a 3 x 3 viewport matrix.
	/// </summary>
	/// <param name="x"> x coordinate of bottom left corner of projection plane </param>
	/// <param name="y"> y coordinate of bottom left corner of projection plane </param>
	/// <param name="width"> width of the projection plane </param>
	/// <param name="height"> height of the projection plane </param>
	/// <returns></returns>
	static std::shared_ptr<Matrix> Viewport(int x, int y,
		int width, int height);

	/// <summary>
	/// Create a view matrix, which corresponds to the 
	/// inverse of camera transformation and is used to transform
	/// the world to camera (eye) coordinates.
	/// </summary>
	/// <param name="eye"> Direction to the camera position </param>
	/// <param name="center"> Direction to the point where the camera is looking </param>
	/// <param name="up">The world up axis</param>
	/// <returns></returns>
	static std::shared_ptr<Matrix> LookAt(std::shared_ptr<Vec4> eye, std::shared_ptr<Vec4> center,
		std::shared_ptr<Vec4> up);

	/// <summary>
	/// Helper function to print a matrix.
	/// </summary>
	/// <param name="matrix"></param>
	static void PrintMatrix(std::shared_ptr<Matrix> matrix);


private:
	unsigned _nrows, _ncols;
	float* _data;
};


#endif