#ifndef SIMPLERES_INTERNAL_H_
#define SIMPLERES_INTERNAL_H_


// Simpleres internal header file, used for non interface headers


typedef struct {
	unsigned int id_start_offset;
	unsigned short id_length;
	unsigned short res_flags;
	unsigned int data_offset;
	unsigned int data_length;
} SMR_ResourceHeader;


// Data corresponding to a resource pack header
typedef struct {
	unsigned short pack_version;
	unsigned short resource_count;
	char *string_section_offset;
	SMR_ResourceHeader *header_section;
} SMR_ResourcePackHeader;


#endif
