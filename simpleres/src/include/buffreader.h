#ifndef SIMPLERES_BUFFREADER_H_
#define SIMPLERES_BUFFREADER_H_


#include <stdio.h>
#include <types.h>


#define BUFFER_SIZE 1024


// Buffer based file reader
// Reads data from the file in chunks in order to limit system calls
typedef struct {
	FILE *f;
	u8 buffered_data[BUFFER_SIZE];
	u16 index;
	u16 bytes_buffered;
} SMR_BuffReader;


// Reads raw count of bytes into the target
u64 SMR_BuffReadRaw(SMR_BuffReader *reader, void *target, u64 bytes);



#endif
