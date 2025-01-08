#include <simpleres/simpleres.h>
#include "include/simpleres_internal.h"
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
		header->header_section[i] = (SMR_ResourceHeader) {
			.data_length = dat_len,
			.data_offset = dat_off,
			.res_flags = flags,
			.id_length = id_len,
			.id_start_offset = id_off
		};
	}

	fclose(pack_file);

	pack->data = data_space;
	pack->data_size = data_size;
	pack->file_name = pack_path;

	return SMR_ERR_OK;
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
