#include <simpleres/simpleres.h>
#include "stdlib.h"

int main() {
	// Open pack
	SMR_ResourcePack pack;
	unsigned int data_size = 2048; // Allocate 2k for testing
	void *data = malloc(data_size);

	int result = SMR_ResourcePackInit(
		&pack,
		"../../../../../test/testpack/test_pack.smr", 
		data,
		data_size
	);

	if(result)
		return result;

	return 0;
}
