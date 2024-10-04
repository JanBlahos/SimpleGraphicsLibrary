#ifndef _CONTEXT_MANAGER_H_
#define _CONTEXT_MANAGER_H_

#include <unordered_map>
#include <memory>

#include "context.h"
#include "sgl.h"

class ContextManager
{
public:
	ContextManager();

	int CreateContext(int& width, int& height, sglEErrorCode& error_code);

	void DestroyContext(int& id, sglEErrorCode& error_code);

	void SetContext(int& id, sglEErrorCode& error_code);

	int GetContext(sglEErrorCode& error_code);

	float* GetColorBufferPtr(void);

	Context* current_context;
private:
	std::vector<std::unique_ptr<Context>> context_container;
	int current_context_idx;
	int next_idx;
	int context_count;
};

#endif
