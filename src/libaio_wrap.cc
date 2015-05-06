#include <libaio.h>
#include "storage.h"
#include "../util/logger.h"
#include "libaio_wrap.h"

int32_t libaio_single_write(io_context_t aio_ctx, int write_fd, char *write_buffer, uint32_t length, uint64_t offset)
{
    int ret = 0;
    struct iocb cb;
    struct iocb *cbs = &cb;
    io_prep_pwrite(&cb, write_fd, write_buffer, length, offset);

    int attempts = 10;
    do
    {
        ret = io_submit(aio_ctx, 1, &cbs);
        if (ret < 0)
        {
            LOG_WARN(logger, "io_submit error, write fd %d, return %d", write_fd, ret);
            if (ret == -EAGAIN && attempts-- > 0)
            {
                usleep(500);
                continue;
            }
            LOG_FATAL(logger, "io_submit got unexpected error");
            return -ERR_AIO;
        }
    }while(false);

    struct io_event e;
    struct timespec t;
    t.tv_sec = 5; // wait 5s
    t.tv_nsec = 0;

    ret = io_getevents(aio_ctx, 1, 1, &e, &t);
    if (ret < 0)
    {
        LOG_FATAL(logger, "io_getevents error, ret %d", ret);
        return -ret;
    }
    else if (ret == 0)
    {
        LOG_FATAL(logger, "io_getevents error, maybe bad block of disk, ret %d", ret);
        return -ERR_AIO;
    }

    return e.res;
}

int32_t libaio_single_read(io_context_t aio_ctx, int read_fd, char *read_buffer, uint32_t length, uint64_t offset)
{
    int ret = 0;
    struct iocb cb;
    struct iocb *cbs = &cb;
    io_prep_pread(cbs, read_fd, read_buffer, length, offset);

    int attempts = 10;
    do
    {
        ret = io_submit(aio_ctx, 1, &cbs);
        if (ret < 0)
        {
            LOG_WARN(logger, "io_submit error, read fd %d, return %d", read_fd, ret);
            if (ret == -EAGAIN && attempts-- > 0)
            {
                usleep(500);
                continue;
            }
            LOG_FATAL(logger, "io_submit got unexpected error");
            return -ERR_AIO;
        }
    }while(false);

    struct io_event e;
    struct timespec t;
    t.tv_sec = 5; // wait 5s
    t.tv_nsec = 0;

    ret = 0;
    ret = io_getevents(aio_ctx, 1, 1, &e, &t);
    if (ret < 0)
    {
        LOG_FATAL(logger, "io_getevents error, ret %d", ret);
        return -ret;
    }
    else if (ret == 0)
    {
        LOG_FATAL(logger, "io_getevents error, maybe bad block of disk, ret %d", ret);
        return -ERR_AIO;
    }

    return e.res;
}

