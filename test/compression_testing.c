#include <stdio.h>
#include <stdlib.h>
#include <simpleres/simpleres.h>

int main() {
	// Open pack
	SMR_ResourcePack pack;
	unsigned int data_size = 10240; // Allocate 10k for testing
	
	void *data = malloc(data_size);

	int result = SMR_ResourcePackInit(
		&pack,
		"../../../../../test/testpack/compressed_pack.smr", 
		data,
		data_size
	);

	// Make sure header has been read correctly
	if(result)
		return result;
/*
	SMR_ResourceSlice uncompressed;
	SMR_GetResource(
		&pack,
		&uncompressed,
		"uncompressed"
	);


	for (int i = 0; i < uncompressed.size; i++) {
		printf("%c", ((char*)uncompressed.data)[i]);
	}
*/
	SMR_ResourceSlice compressed;
	SMR_GetResource(
		&pack,
		&compressed,
		"compressed"
	);


	for (int i = 0; i < compressed.size; i++) {
		printf("%c", ((char*)compressed.data)[i]);
	}

	return 0;
}
