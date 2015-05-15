#include "storage.h"
#include "../util/logger.h"
#include "libaio_wrap.h"

int32_t libaio_single_write(aio_context_t aio_ctx, int write_fd, char *write_buffer, uint32_t length, uint64_t offset)
{
    int ret = 0;
    struct iocb cb;
    struct iocb *cbs = &cb;

    // setup I/O control block
    memset(&cb, 0, sizeof(struct iocb));
    cb.aio_fildes = write_fd;
    cb.aio_lio_opcode = IOCB_CMD_PWRITE;

    // command specific options
    cb.aio_buf = (unsigned long)write_buffer;
    cb.aio_offset = offset;
    cb.aio_nbytes = length;

    int attempts = 10;
    do
    {
        ret = io_submit(aio_ctx, 1, &cbs);
        if (ret != 1)
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
    t.tv_sec = 2; // wait 2s
    t.tv_nsec = 0;

    ret = io_getevents(aio_ctx, 1, 1, &e, &t);
    if (ret < 0)
    {
        LOG_FATAL(logger, "io_getevents error, ret %d", ret);
        return ret;
    }
    else if (ret == 0)
    {
        LOG_FATAL(logger, "io_getevents error, maybe bad block of disk, ret %d", ret);
        return -ERR_AIO;
    }

    return e.res;
}

int32_t libaio_single_read(aio_context_t aio_ctx, int read_fd, char *read_buffer, uint32_t length, uint64_t offset)
{
    int ret = 0;
    struct iocb cb;
    struct iocb *cbs = &cb;

    // setup I/O control block
    memset(&cb, 0, sizeof(struct iocb));
    cb.aio_fildes = read_fd;
    cb.aio_lio_opcode = IOCB_CMD_PREAD;

    // command specific options
    cb.aio_buf = (unsigned long)read_buffer;
    cb.aio_offset = offset;
    cb.aio_nbytes = length;

    int attempts = 10;
    do
    {
        ret = io_submit(aio_ctx, 1, &cbs);
        if (ret != 1)
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
    t.tv_sec = 2; // wait 2s
    t.tv_nsec = 0;

    ret = 0;
    ret = io_getevents(aio_ctx, 1, 1, &e, &t);
    if (ret < 0)
    {
        LOG_FATAL(logger, "io_getevents error, ret %d", ret);
        return ret;
    }
    else if (ret == 0)
    {
        LOG_FATAL(logger, "io_getevents error, maybe bad block of disk, ret %d", ret);
        return -ERR_AIO;
    }

    return e.res;
}

