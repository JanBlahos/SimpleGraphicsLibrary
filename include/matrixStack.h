//---------------------------------------------------------------------------
// matrixStack.h
// header file containing class 
// handling the matrix transformation stack and its switching
//---------------------------------------------------------------------------

#include "matrix.h"
#include <stack>
#include <sgl.h>

class MatrixStack
{
public:
	MatrixStack();

	~MatrixStack();

	/// <summary>
	/// Set whether to use modelView or projection stack
	/// </summary>
	/// <param name="mode"> Decides the current working stack,
	///  use either SGL_MODELVIEW or SGL_PROJECTION</param>
	void SetMode(sglEMatrixMode mode);

	/// <summary>
	/// Return the top matrix from the currently active stack
	/// </summary>
	/// <returns></returns>
	const Matrix& Top();

	/// <summary>
	/// Set the viewport matrix
	/// </summary>
	/// <param name="viewPort"></param>
	void SetViewport(const Matrix& viewPort);

	/// <summary>
	/// Get the viewport matrix
	/// </summary>
	/// <returns></returns>
	const Matrix& GetViewport();

	/// <summary>
	/// Remove the top matrix from the currently active stack
	/// </summary>
	void Pop();

	/// <summary>
	/// Push the given matrix to the top of the currently active stack
	/// </summary>
	/// <param name="matrix"></param>
	void Push(const Matrix& matrix);

	/// <summary>
	/// Create a copy of the matrix on top of the stack and push it onto the stack
	/// </summary>
	void Duplicate();

private:
	// Whether to use the modelView matrix stack or the projection matrix stack
	sglEMatrixMode current_stack_;
	std::stack <Matrix> MV_stack_;
	std::stack <Matrix> P_stack_;
	Matrix viewPort_;
};
