#include <memory.h>
#include <stddef.h>


void SMR_StackInit(SMR_Stack *stack, void *data, size_t size) {
	// Ensure that the start is padded to 8 bytes
	size_t start = (size_t)data;
	size_t start_padding = start % 8;


	stack->start = (char*)start + start_padding;
	stack->end = 0;
	stack->capacity = size - start_padding;
}

void* SMR_StackAlloc(SMR_Stack *stack, size_t size) {
	// Allign size to 8 bytes
	size_t true_size = size + (size % 8);
	
	if (stack->end + true_size < stack->capacity)
		return NULL;

	void *ret = stack->start + stack->end;
	stack->end += true_size;
	return ret;
}

void SMR_StackFree(SMR_Stack *stack, size_t rollback_point) {
	if (rollback_point > stack->capacity)
		return;

	stack->end = rollback_point;
}
