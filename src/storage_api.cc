#include <assert.h>
#include "../util/logger.h"
#include "../include/storage_api.h"
#include "stream_transfer_client_manager.h"
#include "config_opts.h"
#include "free_file_table.h"
#include "index_file.h"
#include "storage.h"
#include "../util/errcode.h"

using namespace util;
using namespace storage;
using namespace std;

Logger *logger = NULL;
IdMap *id_map = NULL;
StreamTransferClientManager *transfer_client_manager = NULL;
FreeFileTable *free_file_table = NULL;
IndexFileManager *index_file_manager = NULL;

uint32_t next_alloc_stream_id = 0;

extern "C"
{

void storage_init()
{
    int32_t ret;
    
    ret = NewLogger(log_dir, &logger);
    assert(ret != 0);

    id_map = new IdMap(logger);
    assert(id_map != NULL);

    transfer_client_manager = new StreamTransferClientManager(logger);
    assert(transfer_client_manager != NULL);
    transfer_client_manager->Init();

    free_file_table = new FreeFileTable(transfer_client_manager);
    assert(free_file_table != NULL);

    index_file_manager = new IndexFileManager(logger, transfer_client_manager, free_file_table);
    assert(index_file_manager != NULL);

    return;
}

int32_t storage_open(char *stream_info, uint32_t size, int flags, uint32_t *id)
{
    assert(stream_info != NULL);
    assert(flags == 0 || flags == 1);

    int32_t ret;
    string key_info(stream_info);

    if (flags == 0)
    {
        ret = transfer_client_manager->Find(key_info);
        if (ret != 0)
        {
            /* 读视频数据时，没有找到视频流，返回失败 */
            return ret;
        }
    }

    ret = id_map.ApplyForFreeId(key_info, id);
    if (ret != 0)
    {
        return ret;
    }

    /* 准备读或写数据结构 */
    ret = transfer_client_manager->Open(key_info, flags);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

int32_t storage_write(const uint32_t id, FRAGMENT_INFO_T *frame_info)
{

}

}

