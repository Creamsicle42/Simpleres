#include <memory.h>
#include <stddef.h>


int SMR_ResHeapInit(SMR_ResHeap *heap, void *data, size_t initial_size) {
	size_t start = (size_t)data;
	size_t true_start = (start % SMR_BLOCKSIZE) + start;
	size_t true_size = initial_size - (true_start - start);
	size_t blocks = true_size / SMR_BLOCKSIZE;
	
	if (blocks < 2) 
		return SMR_MEMERROR_NOT_ENOUGH_MEMORY;

	heap->size = true_size;
	heap->head = (SMR_BlockHeader*)true_start;

	heap->head->data.size = true_size - SMR_BLOCKSIZE;
	heap->head->data.next = NULL;
	heap->head->data.last = NULL;
	heap->head->data.is_free = 1;

	return SMR_MEMERROR_OK;
}


void* SMR_ResHeapAlloc(SMR_ResHeap *heap, size_t size) {
	// Prevent null allocations
	if (!size)
		return NULL;

	// Get number of blocks that will be needed
	// Pad to BLOCKSIZE
	size_t space_needed = (size % SMR_BLOCKSIZE) + size;

	// Itterate over blocks to find first free block of at least the needed size
	SMR_BlockHeader *used_block = heap->head;
	while (
		used_block->data.next != NULL &&
		(
			!used_block->data.is_free ||
			used_block->data.size < space_needed
		) 
	) {
		used_block = (SMR_BlockHeader*)used_block->data.next;
	}

	// Make sure that the block we have settled on is valid
	if (!(used_block->data.is_free && used_block->data.size >= space_needed))
		return NULL;

	// If block is valid, then make a new block at the end of this data chain
	// First make sure ramaining data in this block can fit a new block
	if (used_block->data.size >= space_needed + 2 * SMR_BLOCKSIZE) {
		SMR_BlockHeader* new_block = used_block + 1 + (space_needed / SMR_BLOCKSIZE);
		// Setup connection between new block and old next
		new_block->data.next = used_block->data.next;
		used_block->data.next->data.last = new_block;

		// Setup connection between used block and new
		used_block->data.next = new_block;
		new_block->data.last = used_block;

		// Setup misc data for new block
		new_block->data.size = used_block->data.size - (space_needed + SMR_BLOCKSIZE);
		used_block->data.size = space_needed;
		new_block->data.is_free = 1;
	}

	used_block->data.is_free = 0;

	// Finally return a pointer to used block's data
	return (void*)(used_block + 1);
}


void SMR_ResHeapFree(void *p) {
	SMR_BlockHeader *free_header = (SMR_BlockHeader*)p - 1;
	free_header->data.is_free = 1;

	// Try to combine with next block
	if (free_header->data.next && free_header->data.next->data.is_free) {
		// Increase block size by correct ammount
		free_header->data.size += SMR_BLOCKSIZE + free_header->data.next->data.size;
		// Set next
		free_header->data.next = free_header->data.next->data.next;
		free_header->data.next->data.last = free_header;
	}

	// Try to combine with last block
	if (free_header->data.last && free_header->data.last->data.is_free) {
		// Increase last header size
		free_header->data.last->data.size += SMR_BLOCKSIZE + free_header->data.size;
		free_header->data.next->data.last = free_header->data.last;
		free_header->data.last->data.next = free_header->data.next;
	}
}
