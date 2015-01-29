#include <assert.h>
#include "../util/logger.h"
#include "../include/storage_api.h"
#include "config_opts.h"
#include "free_file_table.h"
#include "index_file.h"
#include "storage.h"
#include "store_client_center.h"

using namespace util;
using namespace storage;
using namespace std;

Logger *logger = NULL;
IdCenter *id_center = NULL;
StoreClientCenter *store_client_center = NULL;
FreeFileTable *free_file_table = NULL;
IndexFileManager *index_file_manager = NULL;

extern "C"
{

void storage_init()
{
    int32_t ret;
    
    ret = NewLogger(log_dir, &logger);
    assert(ret != 0);

    id_center = new IdCenter(logger);
    assert(id_center != NULL);

    store_client_center = new StoreClientCenter(logger);
    assert(store_client_center != NULL);

    free_file_table = new FreeFileTable();
    assert(free_file_table != NULL);

    index_file_manager = new IndexFileManager(logger);
    assert(index_file_manager != NULL);

    return;
}

int32_t storage_open(char *stream_info, uint32_t size, int flags, uint32_t *id)
{
    assert(stream_info != NULL);
    assert(flags == 0 || flags == 1);

    int32_t ret;
    string key_info(stream_info);

    ret = id_map.ApplyForFreeId(key_info, id);
    if (ret != 0)
    {
        return ret;
    }

    /* 准备读或写数据结构 */
    ret = store_client_center->OpenStoreClient(flags, id, key_info);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

int32_t storage_write(const uint32_t id, FRAME_INFO_T *frame_info)
{
    return store_client_center->WriteFrame(id, frame_info);
}

void storage_close(const uint32_t id);
{
    uint32_t ret;
    int flag;
    
    ret = id_center->GetFlag(id, flag);
    if (ret != 0)
    {
        return;
    }

    ret = id_center->ReleaseId(id);
    if (ret != 0)
    {
        return;
    }

    store_client_center->CloseStoreClient(id, flag);

    return;
}

}

