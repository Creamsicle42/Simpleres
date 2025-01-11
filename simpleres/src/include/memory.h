#ifndef SIMPLERES_MEMORY_H_
#define SIMPLERES_MEMORY_H_

#include <stddef.h>

// Internal stack memory implementation


typedef struct {
	char *start; // The begining ot the stack area
	size_t capacity; // The limit of the memory area
	size_t end; // The end of the allocated memory
} SMR_Stack;


void SMR_StackInit(SMR_Stack *stack, void *data, size_t size);
void *SMR_StackAlloc(SMR_Stack *stack, size_t size);
void SMR_StackFree(SMR_Stack *stack, size_t rollback_point);


#endif
