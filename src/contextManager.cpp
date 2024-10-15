
#include "exceptions.h"
#include "contextManager.h"

ContextManager::ContextManager() {
	next_idx = 0;
	context_count = 0;
	current_context_idx = 0;
	current_context = nullptr;
	context_container.reserve(32);
	for (int i = 0; i < 32; ++i) {
		context_container.push_back(nullptr);
	}
}

int ContextManager::CreateContext(int& width, int& height) {
	//First try using all 32 indices
	if (next_idx < 32) {
		try {
			context_container[next_idx++] = std::make_unique<Context>(width, height);
			context_count++;
		}
		catch (const std::bad_alloc& ex) {
			throw OutOfMemoryException("Not enough memory to initialize a new context.");
		}
	//Else find first unused index, if possible
	} else {
		if (context_count > 31) {
			for (int i = 0; i < 32; ++i) {
				if (context_container[i] == nullptr) {
					try {
						context_container[i] = std::make_unique<Context>(width, height);
						context_count++;
					}
					catch (const std::bad_alloc& ex) {
						throw OutOfMemoryException("Not enough memory to initialize a new context.");
					}
				}
			}
		} else {
			throw SGLOutOfResourcesException("Failed to initialize new context. The maximum supported amount of contexts is currently in use.");
		}
	}

	return 0;
};

void ContextManager::DestroyContext(int& id) {
	if (id == current_context_idx) {
		throw SGLInvalidOperationException("Context with the given id is currently in use.");
	}
	if (id > 31 || id < 0 || context_container[id] == nullptr) {
		throw SGLInvalidValueException("Invalid context id.");
	}
	context_container[id] = nullptr;
	context_count--;
};

void ContextManager::SetContext(int& id) {
	if (id > 31 || id < 0 || context_container[id] == nullptr) {
		throw SGLInvalidValueException("Invalid context id.");
		//error_code = SGL_INVALID_VALUE;
		//return;
	}
	current_context_idx = id;
	current_context = context_container[id].get();
};

int ContextManager::GetContext() {
	if (current_context == nullptr) {
		throw SGLInvalidOperationException("No context has been allocated.");
	}
	return current_context_idx;
};

float* ContextManager::GetColorBufferPtr() {
	if (current_context == nullptr) return nullptr;
	return current_context->GetColorBufferPtr();
};
