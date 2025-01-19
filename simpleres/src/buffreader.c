#include <buffreader.h>
#include <types.h>
#include <stdio.h>


void RefreshBuffer(SMR_BuffReader *reader) {
	u64 written = fread(reader->buffered_data, 1, BUFFER_SIZE, reader->f);
	reader->bytes_buffered = written;
	reader->index = 0;
}


u64 SMR_BuffReadRaw(SMR_BuffReader *reader, void *target, u64 bytes) {
	u64 bytes_written = 0;
	char *write_pos = target;

	while (bytes_written < bytes) {
		if (reader->index == reader->bytes_buffered) {
			RefreshBuffer(reader);
		}
		if (reader->bytes_buffered == 0) {
			return bytes_written;
		}
		write_pos[bytes_written++] = reader->buffered_data[reader->index++];
	}
	return bytes_written;
}
