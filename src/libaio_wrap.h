#ifndef STORAGE_LIBAIO_WRAP_H_
#define STORAGE_LIBAIO_WRAP_H_

#include <libaio.h>
#include <sys/syscall.h>

/*
inline int io_setup(unsigned nr, aio_context_t *ctxp)
{
    return syscall(__NR_io_setup, nr, ctxp);
}

inline int io_destroy(aio_context_t ctx)
{
    return syscall(__NR_io_destroy, ctx);
}

inline int io_submit(aio_context_t ctx, long nr, struct iocb **iocbpp)
{
    return syscall(__NR_io_submit, ctx, nr, iocbpp);
}

inline int io_getevents(aio_context_t ctx, long min_nr, long max_nr, struct io_event *events, struct timespec *timeout)
{
    return syscall(__NR_io_getevents, ctx, min_nr, max_nr, events, timeout);
}
*/

int32_t libaio_single_write(io_context_t aio_ctx, int fd, char *write_buffer, uint32_t length, uint64_t offset);
int32_t libaio_single_read(io_context_t aio_ctx, int fd, char *read_buffer, uint32_t length, uint64_t offset);

#endif
