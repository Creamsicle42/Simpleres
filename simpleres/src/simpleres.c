#include <simpleres/simpleres.h>
#include <stddef.h>
#include "include/simpleres_internal.h"
#include "memory.h"
#include "stdio.h"


#define READ_U16(val, file, err_code) {		\
	char buff[2];				\
	if (fread(buff, 1, 2, file) != 2){	\
		fclose(file);			\
		return err_code;		\
	}					\
	for (int i = 0; i < 2; i++)		\
		val = buff[i] + (val << 8);	\
}


#define READ_U32(val, file, err_code) {		\
	char buff[4];				\
	if (fread(buff, 1, 4, file) != 4){	\
		fclose(file);			\
		return err_code;		\
	}					\
	for (int i = 0; i < 4; i++)		\
		val = buff[i] + (val << 8);	\
}

int SMR_ResourcePackInit(
	SMR_ResourcePack *pack,
	const char *pack_path,
	void *data_space,
	unsigned int data_size
) {

	
	// Get file handle
	FILE* pack_file = fopen(pack_path, "r");

	if (!pack_file)
		return SMR_ERR_FILE_NOT_FOUND;

	// Read first four bytes to make sure header is okay
	char header_read[4];
	
	int s_read = fread((void*)header_read, 1, 4, pack_file);
	if (s_read != 4) 
		return SMR_ERR_FILE_CANNOT_READ;

	char ref_header[4] = {'s', 'm', 'p', 'r'};

	for (int i = 0; i < 4; i++) {
		if (header_read[i] != ref_header[i]) {
			fclose(pack_file);
			return SMR_ERR_FILE_HEADER_INVALID;
		}
	}


	// If the file is valid then we go ahead 
	unsigned short file_version;
	READ_U16(file_version, pack_file, SMR_ERR_FILE_HEADER_INVALID)

	unsigned short resource_count;
	READ_U16(resource_count, pack_file, SMR_ERR_FILE_HEADER_INVALID)

	unsigned int id_section_length;
	READ_U32(id_section_length, pack_file, SMR_ERR_FILE_HEADER_INVALID)

	if (id_section_length % 4 != 0) {
		fclose(pack_file);
		return SMR_ERR_FILE_HEADER_INVALID;
	}

	
	// From now on the start of the data space is the header
	SMR_ResourcePackHeader *header = data_space;

	header->resource_count = resource_count;
	header->pack_version = file_version;
	header->string_section_offset = data_space + sizeof(SMR_ResourcePackHeader);
	header->header_section =
		data_space + sizeof(SMR_ResourcePackHeader) + id_section_length;

	// Return an error if there is not enough space to allocate the head
	if (
		sizeof(SMR_ResourcePackHeader)
		+ id_section_length
		+ resource_count * sizeof(SMR_ResourceHeader)
		> data_size
	) {
		fclose(pack_file);
		return SMR_ERR_NOT_ENOUGH_SPACE;
	}

	

	// Read in id section
	int id_read_res = fread(
		(void*)header->string_section_offset,
		1, 
		id_section_length,
		pack_file
	);

	if (id_read_res != id_section_length) {
		fclose(pack_file);
		return SMR_ERR_FILE_CANNOT_READ;
	}

	// Read in header section
	for (int i = 0; i < resource_count; i++) {
		unsigned int id_off;
		unsigned short id_len;
		unsigned short flags;
		unsigned int dat_off;
		unsigned int dat_len;
		unsigned int u_len;
		READ_U32(
			id_off, 
			pack_file, 
			SMR_ERR_FILE_CANNOT_READ
		)
		READ_U16(
			id_len, 
			pack_file, 
			SMR_ERR_FILE_CANNOT_READ
		)
		READ_U16(
			flags, 
			pack_file, 
			SMR_ERR_FILE_CANNOT_READ
		)
		READ_U32(
			dat_off,
			pack_file, 
			SMR_ERR_FILE_CANNOT_READ
		)
		READ_U32(
			dat_len,
			pack_file, 
			SMR_ERR_FILE_CANNOT_READ
		)
		READ_U32(
			u_len,
			pack_file, 
			SMR_ERR_FILE_CANNOT_READ
		)
		header->header_section[i] = (SMR_ResourceHeader) {
			.data_length = dat_len,
			.data_offset = dat_off,
			.res_flags = flags,
			.id_length = id_len,
			.id_start_offset = id_off,
			.data = NULL,
			.uncompressed_size = u_len,
		};
	}

	fclose(pack_file);

	pack->data = data_space;
	pack->data_size = data_size;
	pack->file_name = pack_path;
	
	size_t preamble_size = 
		sizeof(SMR_ResourcePackHeader) +
		id_section_length + 
		(sizeof(SMR_ResourceHeader) * resource_count);

	char* container_data = (char*)data_space + preamble_size;


	SMR_StackInit(&header->data_heap, container_data, data_size - preamble_size);


	return SMR_ERR_OK;
}


int SMR_CmpResName(char *start, int len, const char *compare) {
	for (int i = 0; i < len; i++) {
		if (compare[i] == '\n' && i == len - 1)
			return 0;
		if (compare[i] != start[i])
			return 0;
	}
	return 1;
}


int SMR_GetResource(
	SMR_ResourcePack *pack,
	SMR_ResourceSlice *slice,
	const char *res_id
) {
	SMR_ResourcePackHeader *header = pack->data;

	// Find the resource index
	size_t res_ind = -1;
	for (int i = 0 ; i < header->resource_count; i++) {
		SMR_ResourceHeader *r_header = header->header_section + i;
		if (SMR_CmpResName(header->string_section_offset + r_header->id_start_offset, r_header->id_length, res_id)) {
			res_ind = i;
			break;
		}
	}

	if (res_ind == -1)
		return SMR_ERR_RESOURCE_NOT_FOUND;

	FILE *f = fopen(pack->file_name, "r");

	if (header->header_section[res_ind].data == NULL) {
		if (
			SMR_LoadResourceData(f, header, res_ind)
		) {
			return SMR_ERR_NOT_ENOUGH_SPACE;
		}
	}	

	slice->data = header->header_section[res_ind].data;
	slice->size = header->header_section[res_ind].uncompressed_size;
	
	return SMR_ERR_OK;
}


int SMR_LoadResourceData(FILE *f, SMR_ResourcePackHeader *header, unsigned short id) {
	size_t comp_start = header->header_section[id].data_offset;
	size_t comp_len = header->header_section[id].data_length;
	size_t space_needed = header->header_section[id].uncompressed_size;

	// Make sure we can make enough space
	if (header->data_heap.capacity - header->data_heap.end < space_needed)
		return 1;

	void* data_space = SMR_StackAlloc(&header->data_heap, space_needed);


	// Open the file and seek to the start offset
	fseek(f, comp_start, SEEK_SET);

	// Check compression flags to get proper read func
	if (header->header_section[id].res_flags & SMR_FLAG_LZ77) {
		return SMR_ReadLZ77(f, comp_len, data_space);
	}

	// Read uncompressed if no flags set
	return SMR_ReadUncompressed(f, comp_len, data_space);
}

SMR_ResourceSnapshot SMR_GetSnapshot(SMR_ResourcePack *pack) {
	SMR_ResourcePackHeader *header = pack->data;
	return header->data_heap.end;
}


int SMR_UnloadResources(SMR_ResourcePack *pack, SMR_ResourceSnapshot snapshot) {
	SMR_ResourcePackHeader *header = pack->data;
	SMR_StackFree(&header->data_heap, snapshot);
	return SMR_ERR_OK;
}

int SMR_ReadUncompressed(FILE *f, size_t bytes, char *data) {
	fread(data, 1, bytes, f);
	return 0;
}


unsigned int SMR_ResourcePackGetResourceCount(SMR_ResourcePack *pack) {
	SMR_ResourcePackHeader *header = pack->data;
	return header->resource_count;
}


int SMR_ResourcePackGetResourceName(
	SMR_ResourcePack *pack,
	int resource,
	char *data
) {
	SMR_ResourcePackHeader *header = pack->data;
	if (header->resource_count <= resource)
		return -1;
	unsigned int off = header->header_section[resource].id_start_offset;
	unsigned short len = header->header_section[resource].id_length;
	for (int i = off; i < off + len; i++) {
		char c = header->string_section_offset[i];
		data[i - off] = c;
	}
	data[len] = '\00';

	return len;
}
