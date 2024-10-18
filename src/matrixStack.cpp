
#include "matrixStack.h"
#include "exceptions.h"
#include <iostream>

MatrixStack::MatrixStack() {
	//TODO Matrix default constructor or viewport raw pointer
	current_stack_ = SGL_MODELVIEW;
};

MatrixStack::~MatrixStack() {}

void MatrixStack::SetMode(sglEMatrixMode mode) {
	current_stack_ = mode;
}

sglEMatrixMode MatrixStack::GetMode() {
	return current_stack_;
}

const Matrix& MatrixStack::Top() {
	if (current_stack_ == SGL_MODELVIEW) {
		if (MV_stack_.empty()) {
			throw MatrixStackUnderflowException("No matrices were allocated yet on the Model View stack");
		}
		return MV_stack_.top();
	}
	else {
		if (P_stack_.empty()) {
			throw MatrixStackUnderflowException("No matrices were allocated yet on the Projection stack");
		}
		return P_stack_.top();
	}
}

void MatrixStack::Pop() {
	if (current_stack_ == SGL_MODELVIEW) {
		if (MV_stack_.empty()) {
			throw MatrixStackUnderflowException("No matrices were allocated yet on the Model View stack");
		}
		MV_stack_.pop();
	}
	else {
		if (P_stack_.empty()) {
			throw MatrixStackUnderflowException("No matrices were allocated yet on the Projection stack");
		}
		P_stack_.pop();
	}
}

void MatrixStack::Push(const Matrix& matrix) {
	if (current_stack_ == SGL_MODELVIEW) {
		MV_stack_.push(matrix);
	}
	else {
		P_stack_.push(matrix);
	}
}

void MatrixStack::Duplicate() {
	//TODO maybe recode, at least check again later
	const Matrix& top_matrix = Top();
	auto dimensions = top_matrix.GetDimensions();
	try {
		Push(Matrix(dimensions.first, dimensions.second, top_matrix.GetData()));
	}
	catch (OutOfMemoryException& ex) {
		std::cerr << ex.what() << std::endl;
		throw MatrixStackOverflowException("Out of memory to allocate more matrices on the stack");
	}
}


const Matrix& MatrixStack::GetViewport() {
	return viewPort_;
}

void MatrixStack::SetViewport(const Matrix& viewPort) {
	const std::pair<unsigned int, unsigned int>& dims = viewPort.GetDimensions();
	auto& data = viewPort.GetData();
	viewPort_ = Matrix(dims.first, dims.second, data);
}
