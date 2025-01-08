#include <simpleres/simpleres.h>
#include <stdio.h>
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

	printf("Pack initialized\n");

	char str1[32];
	SMR_ResourcePackGetResourceName(
		&pack,
		0,
		(char*)str1
	);
	printf("Loaded resource %s\n", str1);
	

	char str2[32];
	SMR_ResourcePackGetResourceName(
		&pack,
		1,
		(char*)str2
	);
	printf("Loaded resource %s\n", str2);

	return 0;
}
