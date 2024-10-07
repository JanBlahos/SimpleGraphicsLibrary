
#include "matrixStack.h"
#include "exceptions.h"
#include <iostream>

MatrixStack::MatrixStack() {
	use_modelView_ = true;
};

MatrixStack::~MatrixStack() {
	while (!modelView_stack_.empty()) {
		modelView_stack_.pop();
	}
	while (!projection_stack_.empty()) {
		projection_stack_.pop();
	}
}

void MatrixStack::SetMode(bool useModelView) {
	use_modelView_ = useModelView;
}

std::shared_ptr<Matrix> MatrixStack::Top() {
	if (use_modelView_) {
		if (modelView_stack_.empty()) {
			throw MatrixStackUnderflowException("No matrices were allocated yet on the Model View stack");
		}
		return modelView_stack_.top();
	}
	else {
		if (projection_stack_.empty()) {
			throw MatrixStackUnderflowException("No matrices were allocated yet on the Projection stack");
		}
		return projection_stack_.top();
	}
}
void MatrixStack::Pop() {
	if (use_modelView_) {
		if (modelView_stack_.empty()) {
			throw MatrixStackUnderflowException("No matrices were allocated yet on the Model View stack");
		}
		modelView_stack_.pop();
	}
	else {
		if (projection_stack_.empty()) {
			throw MatrixStackUnderflowException("No matrices were allocated yet on the Projection stack");
		}
		projection_stack_.pop();
	}
}

void MatrixStack::Push(std::shared_ptr<Matrix> matrix) {
	if (use_modelView_) {
		modelView_stack_.push(matrix);
	}
	else {
		projection_stack_.push(matrix);
	}

}

void MatrixStack::Duplicate() {
	auto top_matrix = Top();
	auto dimensions = top_matrix->GetDimensions();
	try {
		std::shared_ptr<Matrix> dup_matrix(new Matrix(dimensions.first, dimensions.second, top_matrix->GetData()));
		Push(dup_matrix);
	}
	catch (OutOfMemoryException& ex) {
		std::cerr << ex.what() << std::endl;
		throw MatrixStackOverflowException("Out of memory to allocate more matrices on the stack");
	}

}

std::shared_ptr<Matrix> MatrixStack::GetViewport() {
	return viewPort_;
}
void MatrixStack::SetViewport(std::shared_ptr<Matrix> viewPort) {
	viewPort_ = viewPort;
}
