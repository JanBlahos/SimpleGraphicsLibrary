
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

int ContextManager::CreateContext(int& width, int& height, sglEErrorCode& error_code) {
	//First try using all 32 indices
	if (next_idx < 32) {
		try {
			context_container[next_idx++] = std::make_unique<Context>(width, height);
			context_count++;
		}
		catch (const std::bad_alloc& ex) {
			error_code = SGL_OUT_OF_MEMORY;
			return 0;
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
						error_code = SGL_OUT_OF_MEMORY;
						return 0;
					}
				}
			}
		} else {
			error_code = SGL_OUT_OF_RESOURCES;
		}
	}

	return 0;
};

void ContextManager::DestroyContext(int& id, sglEErrorCode& error_code) {
	if (id == current_context_idx) {
		error_code = SGL_INVALID_OPERATION;
		return;
	}
	if (id > 31 || id < 0 || context_container[id] == nullptr) {
		error_code = SGL_INVALID_VALUE;
		return;
	}
	context_container[id] = nullptr;
	context_count--;
};

void ContextManager::SetContext(int& id, sglEErrorCode& error_code) {
	if (id > 31 || id < 0 || context_container[id] == nullptr) {
		error_code = SGL_INVALID_VALUE;
		return;
	}
	current_context_idx = id;
	current_context = context_container[id].get();
};

int ContextManager::GetContext(sglEErrorCode& error_code) {
	if (current_context == nullptr) {
		error_code = SGL_INVALID_OPERATION;
		return -1;
	}
	return current_context_idx;
};

float* ContextManager::GetColorBufferPtr(void) {
	if (current_context == nullptr) return nullptr;
	return current_context->GetColorBufferPtr();
};
