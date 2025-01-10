#ifndef SIMPLERES_INTERNAL_H_
#define SIMPLERES_INTERNAL_H_

#include <stddef.h>
#include <stdio.h>
#include <memory.h>

// Simpleres internal header file, used for non interface headers


typedef struct {
	unsigned int id_start_offset;
	unsigned short id_length;
	unsigned short res_flags;
	unsigned int data_offset;
	unsigned int data_length;
	unsigned int uncompressed_size;
	void* data;
} SMR_ResourceHeader;


// Data corresponding to a resource pack header
typedef struct {
	unsigned short pack_version;
	unsigned short resource_count;
	char *string_section_offset;
	SMR_ResourceHeader *header_section;
	SMR_Stack data_heap;
} SMR_ResourcePackHeader;


int SMR_LoadResourceData(FILE *f, SMR_ResourcePackHeader *header, unsigned short id);
int SMR_ReadUncompressed(FILE *f, size_t bytes, char *data);
int SMR_ReadLZ77(FILE *f, size_t bytes, char *data);

#endif
