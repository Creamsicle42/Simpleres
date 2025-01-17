#include <simpleres/simpleres.h>
#include <stddef.h>
#include "include/simpleres_internal.h"
#include "memory.h"
#include "stdio.h"


#define READ_U16(val, file, err_code, err_text) {		\
	char buff[2];				\
	if (fread(buff, 1, 2, file) != 2){	\
		fclose(file);			\
		printf(err_text);\
		return err_code;		\
	}					\
	for (int i = 0; i < 2; i++)		\
		val = buff[i] + (val << 8);	\
}


#define READ_U32(val, file, err_code, err_text) {		\
	char buff[4];				\
	if (fread(buff, 1, 4, file) != 4){	\
		fclose(file);			\
		printf(err_text);		\
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
	FILE* pack_file = fopen(pack_path, "rb");

	if (!pack_file)
		return SMR_ERR_FILE_NOT_FOUND;

	

	union Reader{
		struct {
		char id[4];
		unsigned short version;
		unsigned short res_count;
		unsigned int id_section_len;
		} str;
		char byt[12];
	 };

	union Reader reader;
	if (fread(&reader.byt, sizeof(union Reader), 1, pack_file) != 1) {
		fclose(pack_file);
		printf("Was not able to read entire header, file must be at least %u bytes\n", sizeof(union Reader));
		return SMR_ERR_FILE_CANNOT_READ;
	}

	char ref_header[4] = {'s', 'm', 'p', 'r'};

	for (int i = 0; i < 4; i++) {
		if (reader.str.id[i] != ref_header[i]) {
			fclose(pack_file);
			printf("File must start with 'smpr'\n");
			return SMR_ERR_FILE_HEADER_INVALID;
		}
	}



	if (reader.str.id_section_len % 4 != 0) {
		fclose(pack_file);
		printf("ID section must be padded to a multiple of four, currently is %d\n", reader.str.id_section_len);
		return SMR_ERR_FILE_HEADER_INVALID;
	}

	
	// From now on the start of the data space is the header
	SMR_ResourcePackHeader *header = data_space;

	header->resource_count = reader.str.res_count;
	header->pack_version = reader.str.version;
	header->string_section = data_space + sizeof(SMR_ResourcePackHeader);
	header->header_section =
		data_space + sizeof(SMR_ResourcePackHeader) + reader.str.id_section_len;

	// Return an error if there is not enough space to allocate the head
	if (
		sizeof(SMR_ResourcePackHeader)
		+ reader.str.id_section_len 
		+ reader.str.res_count * sizeof(SMR_ResourceHeader)
		> data_size
	) {
		fclose(pack_file);
		return SMR_ERR_NOT_ENOUGH_SPACE;
	}

	

	// Read in id section
	int id_read_res = fread(
		(void*)header->string_section,
		1, 
		reader.str.id_section_len,
		pack_file
	);

	if (id_read_res != reader.str.id_section_len) {
		fclose(pack_file);
		return SMR_ERR_FILE_CANNOT_READ;
	}

	// Read in header section
	for (int i = 0; i < reader.str.res_count; i++) {
		struct ResReader{
			unsigned int id_start;
			unsigned short id_len;
			unsigned short flags;
			unsigned int data_start;
			unsigned int comp_len;
			unsigned int uncomp_len;
		} res;
		
		if (fread(&res, sizeof(struct ResReader), 1, pack_file) != 1) {
			fclose(pack_file);
			return SMR_ERR_FILE_CANNOT_READ;
		}
		header->header_section[i] = (SMR_ResourceHeader) {
			.data = NULL,
			.id_length = res.id_len,
			.res_flags = res.flags,
			.data_length = res.comp_len,
			.uncompressed_size = res.uncomp_len,
			.data_offset = res.data_start,
			.id_start_offset = res.id_start
			
		};
	}

	fclose(pack_file);

	pack->data = data_space;
	pack->data_size = data_size;
	pack->file_name = pack_path;
	
	size_t preamble_size = 
		sizeof(SMR_ResourcePackHeader) +
		reader.str.id_section_len + 
		(sizeof(SMR_ResourceHeader) * reader.str.res_count);

	char* container_data = (char*)data_space + preamble_size;


	SMR_StackInit(&header->data_heap, container_data, data_size - preamble_size);


	return SMR_ERR_OK;
}


int SMR_CmpResName(char *start, int len, const char *compare) {
	for (int i = 0; i < len - 1; i++) {
		//printf("Checking char #%d\n", i);
		//printf("- Ref %c\n", compare[i]);
		//printf("- Start %c\n", start[i]);
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
		if (SMR_CmpResName(
			header->string_section + r_header->id_start_offset,
			r_header->id_length,
			res_id
		)) {
			res_ind = i;
			break;
		}
	}

	if (res_ind == -1)
		return SMR_ERR_RESOURCE_NOT_FOUND;


	FILE *f = fopen(pack->file_name, "rb");

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
	
	if (data_space == NULL)
		return 1;

	//printf("Starting read form pos %u\n", comp_start);
	// Open the file and seek to the start offset
	fseek(f, comp_start, SEEK_SET);

	header->header_section[id].data = data_space;
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

int SMR_ReadLZ77(FILE *f, size_t bytes, char *data) {
	struct LZ77Packet{
		unsigned short lookback;
		char repeat;
		char data;
	} ;
	size_t remaining_bytes = bytes;
	char *write_pos = data;
	char test;
	fread(&test, 1, 1, f);
	//printf("Test read is 0x%02X\n",test);
	fseek(f, -1, SEEK_CUR);
	while (remaining_bytes > 0) {
		// Read in a 16 bit value
		union {
			char b[2];
			unsigned short lb;
		} r;
		unsigned short lookback;
		fread(&r.b[0], 1, 1, f);
		fread(&r.b[1], 1, 1, f);
		lookback = r.lb;
		//printf("0x%04X - ", lookback);
		// If value is less than 256 then it is a literal
		if (lookback < 256) {
			char ch = lookback;
			*write_pos = ch;
			//printf("- %c", ch);
			write_pos++;
			remaining_bytes -= 2;
			//printf(" - %u\n", remaining_bytes);
			continue;
		}

		// Otherwise it's a lookback value and it should be used as such
		lookback -= 255;
		unsigned char run;
		fread(&run, sizeof(char), 1, f);
		remaining_bytes -= 3;
		//printf("Reading %u lookback bytes\n", lookback);
		for (int i = 0; i < run; i++) {
			*write_pos = *(write_pos - lookback);
			//printf("- %c", *(write_pos - lookback));
			write_pos++;
		}

	}

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
		char c = header->string_section[i];
		data[i - off] = c;
	}
	data[len] = '\00';

	return len;
}
