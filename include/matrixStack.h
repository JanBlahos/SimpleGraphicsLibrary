//---------------------------------------------------------------------------
// matrixStack.h
// header file containing class 
// handling the matrix transformation stack and its switching
//---------------------------------------------------------------------------

#include "matrix.h"
#include <stack>

class MatrixStack
{
public:
	MatrixStack();

	~MatrixStack();

	/// <summary>
	/// Set whether to use modelView or projection stack
	/// </summary>
	/// <param name="useModelView"> True if modelView stack is to be used,
	/// false if projection stack is to be used</param>
	void SetMode(bool useModelView);

	/// <summary>
	/// Return the top matrix from the currently active stuck
	/// </summary>
	/// <returns></returns>
	std::shared_ptr<Matrix> Top();

	/// <summary>
	/// Set the viewport matrix
	/// </summary>
	/// <param name="viewPort"></param>
	void SetViewport(std::shared_ptr<Matrix> viewPort);

	/// <summary>
	/// Get the viewport matrix
	/// </summary>
	/// <returns></returns>
	std::shared_ptr<Matrix> GetViewport();

	/// <summary>
	/// Remove the top matrix from the currently active stack
	/// </summary>
	void Pop();

	/// <summary>
	/// Push the given matrix to the top of the currently active stack
	/// </summary>
	/// <param name="matrix"></param>
	void Push(std::shared_ptr<Matrix> matrix);

	/// <summary>
	/// Create a copy of the matrix on top of the stack and push it onto the stack
	/// </summary>
	void Duplicate();

private:
	// Whether to use the modelView matrix stack or the projection matrix stack
	bool use_modelView_;
	std::stack <std::shared_ptr<Matrix>> modelView_stack_;
	std::stack <std::shared_ptr<Matrix>> projection_stack_;
	Matrix *viewPort_;
};
