#ifndef SIMPLERES_SIMPLERES_H_
#define SIMPLERES_SIMPLERES_H_

/**
* Simpleres main header.
*/


// Includes
#include <stdio.h>

// -----------------
// Constants
// -----------------

#define SMR_ERR_OK 0
#define SMR_ERR_FILE_NOT_FOUND 1
#define SMR_ERR_FILE_CANNOT_READ 2
#define SMR_ERR_FILE_HEADER_INVALID 3
#define SMR_ERR_NOT_ENOUGH_SPACE 4

// -----------------
// Struct Definition
// -----------------

/**
 * Struct representing a slice of resource data within the packs memory space
*/
typedef struct {
	void* data; // Pointer to the start of the data
	unsigned int size; // Size of the data
} SMR_ResourceSlice;


/**
 * Handle to a resource within a pack
*/
typedef struct {
	unsigned short resource_id;
} SMR_ResourceHandle;


/**
* Handle to a loaded resource pack
*
* Pack internal structure is intended to be opaque, definition here is only included so that they can be created directly on the stack.
*/
typedef struct {
	const char *file_name; // Name of the pack file
	void *data; // Pointer to the data used by the pack
	unsigned int data_size; // Size of data allocated for pack use
} SMR_ResourcePack;

// ---------------
// Func Definition
// ---------------


/**
 * Initializes a resource pack struct
 *
 * @param *pack The resource pack struct being initialized
 * @param *pack_path Path to the resource pack file on disk
 * @param *data_space Data space that will be used by the pack
 * @param data_size The size of the data space that has been allocated
 * @return A SMR error code
*/
int SMR_ResourcePackInit(
	SMR_ResourcePack *pack,
	const char *pack_path,
	void *data_space,
	unsigned int data_size
);


/**
 * Gets a resource handle, loading the resource from disk if needed
 *
 * @param *pack The pack that will be searched for the resource handle
 * @param *handle The handle that will be set to be the resource handle
 * @param *res_id The resource id to get
 * @return A SMR error code
*/
int SMR_GetResourceHandle(
	SMR_ResourcePack *pack,
	SMR_ResourceHandle *handle,
	const char *res_id
);


/**
 * Uses a resource handle to get a pointer to data in memory
 * Slices returned may be invalidated in the future, so make sure to call this before that can happen
 *
 * @param *pack The pack that has the resource
 * @param *handle The handle to the desired resource
 * @param *slice the slice that will be filled in
 * @return A SMR error code
*/
int SMR_GetResourceSlice(
	SMR_ResourcePack *pack,
	SMR_ResourceHandle *handle,
	SMR_ResourceSlice *slice
);


/*
 * Returns a resource handle that is no longer needed
 *
 * @param *pack The pack that has the resource
 * @param *handle The handle being returned
*/
void SMR_ReturnResourceHandle(
	SMR_ResourcePack *pack,
	SMR_ResourceHandle *handle
);

#endif
