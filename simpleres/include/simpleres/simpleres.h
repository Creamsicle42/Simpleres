#ifndef SIMPLERES_SIMPLERES_H_
#define SIMPLERES_SIMPLERES_H_

/**
* Simpleres main header.
*/


// Includes
#include <stddef.h>
#include <stdio.h>

// -----------------
// Constants
// -----------------

#define SMR_ERR_OK 0
#define SMR_ERR_FILE_NOT_FOUND 1
#define SMR_ERR_FILE_CANNOT_READ 2
#define SMR_ERR_FILE_HEADER_INVALID 3
#define SMR_ERR_NOT_ENOUGH_SPACE 4
#define SMR_ERR_RESOURCE_NOT_FOUND 5


#define SMR_FLAG_LZ77 1

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



// A snapshot of tine of the resourece manager which can be rollbacked to to unload resources
typedef size_t SMR_ResourceSnapshot;


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
int SMR_GetResource(
	SMR_ResourcePack *pack,
	SMR_ResourceSlice *slice,
	const char *res_id
);


/**
 * Gets a snapshot that can be rollbacked to to unload resources.
 *
 * @param pack The pack to get a snapshot of.
 * @return A snapshot.
 */
SMR_ResourceSnapshot SMR_GetSnapshot(SMR_ResourcePack *pack);


/**
 *  Unloads all resources loaded after a given snapshot.
 *
 *  @param *pack The pack to unload resources in.
 *  @param snapshot The snapshot to unload resources to.
 *  @return A SMR error code
 */
int SMR_UnloadResources(
	SMR_ResourcePack *pack,
	SMR_ResourceSnapshot snapshot
);

/**
 * Get the number of resources in a pack
*/
unsigned int SMR_ResourcePackGetResourceCount(
	SMR_ResourcePack *pack
);


/**
* Gets the name of a resource.
*
* Fills in a char array with a null terminated string containing a
* resource id.
*
* WARNING: Not intended to be data safe, onely useful for testing
*
* @param *pack Pack to get data about
* @param resource Numerical index of the given resource
* @param **data Pointer to a string that will be filled in
* @return The length of the data string
*/
int SMR_ResourcePackGetResourceName(
	SMR_ResourcePack *pack,
	int resource,
	char *data
);

#endif
