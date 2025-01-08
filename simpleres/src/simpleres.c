#include <simpleres/simpleres.h>
#include "include/simpleres_internal.h"
#include "stdio.h"

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
	if (
		fread((void*)&file_version, sizeof(short), 1, pack_file) 
		!= sizeof(short)
	) {
		fclose(pack_file);
		return SMR_ERR_FILE_HEADER_INVALID;
	}

	unsigned short resource_count;
	if (
		fread((void*)&resource_count, sizeof(short), 1, pack_file)
		!= sizeof(short)
	) {
		fclose(pack_file);
		return SMR_ERR_FILE_HEADER_INVALID;
	}

	unsigned int id_section_length;
	if (
		fread((void*)&id_section_length, sizeof(int), 1, pack_file)
		!= sizeof(int)
	) {
		fclose(pack_file);
		return SMR_ERR_FILE_HEADER_INVALID;
	}

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
		+ resource_count * sizeof(SMR_ResourcePackHeader)
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
	int res_headers_read = fread(
		(void*)header->header_section,
		sizeof(SMR_ResourceHeader),
		resource_count,
		pack_file
	);
	if (res_headers_read != resource_count) {
		fclose(pack_file);
		return SMR_ERR_FILE_CANNOT_READ;
	}


	fclose(pack_file);

	return SMR_ERR_OK;
}


unsigned int SMR_ResourcePackGetResourceCount(SMR_ResourcePack *pack) {
	return ((SMR_ResourcePackHeader*)pack->data)->resource_count;
}


int SRM_ResourcePackGetResourceName(
	SMR_ResourcePack *pack,
	int resource,
	char **data
) {
	SMR_ResourcePackHeader* header = pack->data;
	if (header->resource_count <= resource)
		return -1;

	unsigned int off = header->header_section[resource].id_start_offset;
	unsigned int len = header->header_section[resource].id_length;

	for (int i = 0; i < len; i++)
		(*data)[i] = header->string_section_offset[off + i];
	(*data)[len] = '\n';

	return len;
}
