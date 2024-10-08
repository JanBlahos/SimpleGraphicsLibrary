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

	int CreateContext(int& width, int& height);

	void DestroyContext(int& id);

	void SetContext(int& id);

	int GetContext();

	float* GetColorBufferPtr();

	Context* current_context;
private:
	std::vector<std::unique_ptr<Context>> context_container;
	int current_context_idx;
	int next_idx;
	int context_count;
};

#endif
