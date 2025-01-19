#ifndef SIMPLERES_INTERNAL_H_
#define SIMPLERES_INTERNAL_H_

#include <stddef.h>
#include <stdio.h>
#include <memory.h>
#include <types.h>

// Simpleres internal header file, used for non interface headers


typedef struct {
	u32 id_start_offset;
	u16 id_length;
	u16 res_flags;
	u32 data_offset;
	u32 data_length;
	u32 uncompressed_size;
	void* data;
} SMR_ResourceHeader;


// Data corresponding to a resource pack header
typedef struct {
	u16 pack_version;
	u16 resource_count;
	char *string_section;
	SMR_ResourceHeader *header_section;
	SMR_Stack data_heap;
} SMR_ResourcePackHeader;


int SMR_LoadResourceData(FILE *f, SMR_ResourcePackHeader *header, unsigned short id);
int SMR_ReadUncompressed(FILE *f, size_t bytes, char *data);
int SMR_ReadLZ77(FILE *f, size_t bytes, char *data);

#endif
