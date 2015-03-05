#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../util/logger.h"
#include "../include/storage_api.h"
#include "config_opts.h"
#include "free_file_table.h"
#include "index_file.h"
#include "../include/storage.h"
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

int32_t storage_get_disk_info(char *disk, DISK_INFO_T *disk_info)
{
    char disk_mount_path[64] = {0};
    char file_count_str[32] = {0};
    
    memset(disk_info, 0, sizeof(DISK_INFO_T));
    snprintf(disk_info->name, 31, "%s", disk);
    
    snprintf(disk_mount_path, 63, "/jovision/%s/file_count", disk);
    int fd = open(disk_mount_path, O_RDONLY);
    if (fd == -1)
    {
        strcpy(disk_info->status, "Uninitialized");
        return 0;
    }

    if (pread(fd, file_count_str, 32, 0) <= 0)
    {
        strcpy(disk_info->status, "Uninitialized");
        return 0;
    }
    close(fd);

    int file_count = atoi(file_count_str);
    disk_info->capacity = file_count * 256;
    strcpy(disk_info->status, "Initialized");

    return 0;
}

int32_t storage_formate_disk(char *disk)
{
    char command[256] = {0};
    FILE *fp = NULL;

    snprintf(command, 255, "/jovision/shell/disk_format/format_one_disk.sh %s", disk);
    fp = popen(command, "r");
    if (fp == NULL)
    {
        return -1;
    }

    char *result_buffer = new char[2048];
    memset(result_buffer, 0, 2048);
    while(fgets(result_buffer, 2048, fp) != NULL)
    {
        
    }

    return pclose(fp);
}

void storage_init()
{
    int32_t ret;
    
    ret = NewLogger("/tmp/storage.log", &logger);
    assert(ret == 0);

    id_center = new IdCenter(logger);
    assert(id_center != NULL);

    store_client_center = new StoreClientCenter(logger);
    assert(store_client_center != NULL);
    store_client_center->Init();

    free_file_table = new FreeFileTable(logger);
    assert(free_file_table != NULL);

    index_file_manager = new IndexFileManager(logger);
    assert(index_file_manager != NULL);
    index_file_manager->Init();

    return;
}

int32_t storage_open(char *stream_info, uint32_t size, int flags, uint32_t *id)
{
    assert(stream_info != NULL);
    assert(flags == 0 || flags == 1);

    int32_t ret;
    string key_info(stream_info);

    ret = id_center->ApplyForId(key_info, flags, id);
    if (ret != 0)
    {
        return ret;
    }

    ret = store_client_center->Open(flags, *id, key_info);
    if (ret != 0)
    {
        id_center->ReleaseId(*id);
        return ret;
    }

    return 0;
}

int32_t storage_write(const uint32_t id, FRAME_INFO_T *frame_info)
{
    return store_client_center->WriteFrame(id, frame_info);
}

int32_t storage_list_record_fragments(const uint32_t id, const UTIME_T *start, const UTIME_T *end, 
        FRAGMENT_INFO_T **frag_info, uint32_t *count)
{
    int32_t ret;
    deque<FRAGMENT_INFO_T*> frag_info_queue;
    UTime start_time(start->seconds, start->nseconds);
    UTime end_time(end->seconds, end->nseconds);

    ret = store_client_center->ListRecordFragments(id, start_time, end_time, frag_info_queue);
    if (ret != 0)
    {
        return ret;
    }
    
    *count = frag_info_queue.size();
    if (*count == 0)
    {
        frag_info = NULL;
        return 0;
    }

    frag_info = new FRAGMENT_INFO_T*[*count];
    assert(frag_info != NULL);

    for (int i = 0; i < *count; i++)
    {
        frag_info[i] = frag_info_queue.front();
        frag_info_queue.pop_front();
    }

    return 0;
}

int32_t storage_seek(const uint32_t id, const UTIME_T *stamp)
{
    UTime time_stamp(stamp->seconds, stamp->nseconds);
    return store_client_center->SeekRead(id, time_stamp);
}

int32_t storage_read(const uint32_t id, FRAME_INFO_T *frame_info)
{
    return store_client_center->ReadFrame(id, frame_info);
}

void storage_close(const uint32_t id)
{
    int32_t ret;
    int flag;
    
    ret = id_center->GetFlag(id, flag);
    if (ret != 0)
    {
        return;
    }

    id_center->ReleaseId(id);
    ret = store_client_center->Close(id, flag);

    return;
}

void storage_shutdown()
{
    id_center->Shutdown();
    delete id_center;
    id_center = NULL;

    store_client_center->Shutdown();
    delete store_client_center;
    store_client_center = NULL;

    free_file_table->Shutdown();
    delete free_file_table;
    free_file_table = NULL;

    index_file_manager->Shutdown();
    delete index_file_manager;
    index_file_manager = NULL;

    delete logger;
    logger = NULL;
    
    return;
}

}

