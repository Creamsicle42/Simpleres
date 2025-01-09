#ifndef SIMPLERES_MEMORY_H_
#define SIMPLERES_MEMORY_H_

#include <stddef.h>

// Internal heap memory implementation

#define SMR_BLOCKSIZE 32

#define SMR_MEMERROR_OK 0
#define SMR_MEMERROR_NOT_ENOUGH_MEMORY 1

typedef union bHeader SMR_BlockHeader;

// Data type representing a memory header, alligned to 16 bytes
union bHeader {
	struct {
		size_t size;
		unsigned is_free;
		SMR_BlockHeader *next;
		SMR_BlockHeader *last;
	} data;
	char ALLIGN[SMR_BLOCKSIZE];
};


// Struct representing a heap style memory allocation
typedef struct {
	size_t size; // Raw size of allocated memory
	SMR_BlockHeader *head; // Head of block list
} SMR_ResHeap;



int SMR_ResHeapInit(SMR_ResHeap *heap, void *data, size_t initial_size);
void *SMR_ResHeapAlloc(SMR_ResHeap *heap, size_t size);
void SMR_ResHeapFree(void *p);



#endif
