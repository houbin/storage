#ifndef STORAGE_LIBAIO_WRAP_H_
#define STORAGE_LIBAIO_WRAP_H_
#include <libaio.h>

int32_t libaio_single_write(io_context_t aio_ctx, int fd, char *write_buffer, uint32_t length, uint64_t offset);
int32_t libaio_single_read(io_context_t aio_ctx, int fd, char *read_buffer, uint32_t length, uint64_t offset);

#endif
