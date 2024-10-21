//---------------------------------------------------------------------------
// contextManager.h
// header for ContextManager class
//---------------------------------------------------------------------------

#ifndef _CONTEXT_MANAGER_H_
#define _CONTEXT_MANAGER_H_

#include <unordered_map>
#include <memory>

#include "context.h"
#include "sgl.h"

#define MAX_CONTEXT_COUNT 32

/// <summary>
/// Class for handling contexts
/// </summary>
class ContextManager
{
public:
	/// <summary>
	/// Default ctor
	/// </summary>
	ContextManager();

	/// <summary>
	/// Initializes a single context
	/// </summary>
	/// <param name="width">target width</param>
	/// <param name="height">target height</param>
	/// <returns></returns>
	int CreateContext(int& width, int& height);

	/// <summary>
	/// Destroys context with the given id
	/// </summary>
	/// <param name="id">id of context to be destroyed</param>
	void DestroyContext(int& id);

	/// <summary>
	/// Sets current context via given id
	/// </summary>
	/// <param name="id">id of context to switch to</param>
	void SetContext(int& id);

	/// <summary>
	/// Returns id of the current context
	/// </summary>
	/// <returns>Context id</returns>
	int GetContext();

	/// <summary>
	/// Returns color buffer
	/// </summary>
	/// <returns>Ptr to color buffer of the current context</returns>
	float* GetColorBufferPtr();

	Context* current_context;
private:
	std::vector<std::unique_ptr<Context>> context_container;
	int current_context_idx;
	int next_idx;
	int context_count;
};

#endif
