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

	// Make sure header has been read correctly
	if(result)
		return result;

	int res_count = SMR_ResourcePackGetResourceCount(&pack);
	// Make sure pack has expected resource count
	if (res_count != 2) {
		printf("Resource count is actually %d", res_count);
		return 1;
	}

	return 0;
}
