#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include "../util/logger.h"
#include "../util/config.h"
#include "../include/storage_api.h"
#include "config_opts.h"
#include "free_file_table.h"
#include "index_file.h"
#include "store_client_center.h"
#include "watchdog_client.h"

using namespace util;
using namespace storage;
using namespace std;

Logger *logger = NULL;
IdCenter *id_center = NULL;
StoreClientCenter *store_client_center = NULL;
FreeFileTable *free_file_table = NULL;
IndexFileManager *index_file_manager = NULL;
Config *config = NULL;
RecordRecycle *record_recycle = NULL;
WatchDogClient *dog_client = NULL;

#define WATCHDOG_SERVER_PORT 6010

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

    int32_t record_info_index_length = sizeof(RecordFileInfo);
    int32_t frag_info_index_length = sizeof(RecordFragmentInfo);

    snprintf(command, 255, "/jovision/shell/disk_format/format_one_disk.sh %s %d %d", disk, record_info_index_length, frag_info_index_length);
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

void storage_handle_signal(int signum)
{ 
    if (signum == SIGUSR1)
    {
        string log_dir("/jovision/storage.conf");
        if (!config->FileExist(log_dir.c_str()))
        {
            return;
        }

        config->ReadFile(log_dir.c_str());
        int log_level = config->Read("log_level", log_level);
        logger->SetLogLevel((Logger::LogLevel)log_level);

        LOG_INFO(logger, "logger set log level to %d", log_level);
    }
    else if (signum == SIGUSR2)
    {
        store_client_center->Dump();
        record_recycle->Dump();
        free_file_table->Dump();
        index_file_manager->Dump();
    }

    return;
}

void storage_register_signal(int signum)
{
    struct sigaction act;
    
    memset(&act, 0, sizeof(act));
    act.sa_handler = storage_handle_signal;
    act.sa_flags = 0;
    
    int ret = sigaction(signum, &act, NULL);
    assert(ret == 0);

    return;
}

void storage_get_process_info(string &process_path, string &process_dir)
{
    char buffer[1024] = {0};
    int ret = 0;
    int fd;
    int offset = 0;
    char cmdline_path[32] = {0};
    char cmdline_info[256] = {0};
    char *start = NULL;
    string whole_process_path;

    int cnt = readlink("/proc/self/exe", buffer, 1023);
    assert(cnt > 0 && cnt <= 1023);

    snprintf(cmdline_path, 31, "/proc/self/cmdline");
    fd = open(cmdline_path, O_RDONLY);
    if (fd < 0)
    {
        LOG_WARN(logger, "open cmdline file error, error msg %s", strerror(errno));
        assert(fd >= 0);
    }

    ret = read(fd, cmdline_info, 255);
    assert (ret > 0);

    // skip relative exe path
    start = &(cmdline_info[offset]);
    offset = strlen(start) + 1;

    // assign absolute exe path
    whole_process_path.assign(buffer);

    while (offset < ret)
    {
        start = &(cmdline_info[offset]);
        whole_process_path.append(" ");
        whole_process_path.append(start);

        offset = offset + strlen(start) + 1;
    }
    
    process_path = whole_process_path;

    int i = 0;
    for (i = cnt; i >= 0; i--)
    {
        if (buffer[i] == '/')
        {
            buffer[i+1] = 0;
            break;
        }
    }

    process_dir.assign(buffer);

    LOG_INFO(logger, "process absolute path %s, process absolute dir %s", process_path.c_str(), process_dir.c_str());

    return ;
}

void storage_start_watchdog(string process_path, int lost_threshold)
{
    struct sockaddr_in server_addr;
    string reboot_path;

    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_addr.sin_port = htons(WATCHDOG_SERVER_PORT);

    dog_client = new WatchDogClient(logger, server_addr, process_path, getpid(), lost_threshold);
    assert(dog_client != NULL);

    dog_client->Init();

    return;
}

void storage_init()
{
    int32_t ret;
    string log_dir = "/var/log/storage/storage.log";
    int32_t log_level = 2;

    config = new Config("/jovision/storage.conf");
    assert(config != NULL);

    log_dir = config->Read("log_dir", log_dir);
    log_level = config->Read("log_level", log_level);
    ret = NewLogger(log_dir.c_str(), &logger);
    logger->SetLogLevel((Logger::LogLevel)log_level);
    assert(ret == 0);

    id_center = new IdCenter(logger);
    assert(id_center != NULL);

    store_client_center = new StoreClientCenter(logger);
    assert(store_client_center != NULL);
    store_client_center->Init();

    record_recycle = new RecordRecycle(logger, store_client_center);
    assert(record_recycle != NULL);
    record_recycle->Init();

    free_file_table = new FreeFileTable(logger);
    assert(free_file_table != NULL);

    index_file_manager = new IndexFileManager(logger);
    assert(index_file_manager != NULL);
    index_file_manager->Init();

    // get path and dir of exe
    string process_path;
    string process_dir;
    storage_get_process_info(process_path, process_dir);

    // change current work dir
    chdir(process_dir.c_str());

    int lost_threshold = 0;
    lost_threshold = config->Read("watchdog_lost_threshold", lost_threshold);
    assert(lost_threshold > 0);
    storage_start_watchdog(process_path, lost_threshold);

    storage_register_signal(SIGUSR1);
    storage_register_signal(SIGUSR2);

    LOG_INFO(logger, "storage init ok");

    return;
}

int32_t storage_open(char *stream_info, uint32_t size, int flags, int32_t *id)
{
    assert(stream_info != NULL);
    assert(flags == 0 || flags == 1);
    LOG_INFO(logger, "storage open stream %s, flag %d", stream_info, flags);

    int32_t ret;
    string key_info(stream_info);

    ret = id_center->ApplyForId(key_info, flags, id);
    if (ret != 0)
    {
        LOG_WARN(logger, "apply id error, stream %s, flag %d, ret %d", stream_info, flags, ret);
        return ret;
    }

    ret = store_client_center->Open(flags, *id, key_info);
    if (ret != 0)
    {
        LOG_WARN(logger, "open error, stream info [%s], flag %d, id %d, ret %d", stream_info, flags, *id, ret);
        id_center->ReleaseId(*id);
        return ret;
    }

    LOG_INFO(logger, "storage open stream %s ok, flag %d, id %d", stream_info, flags, *id);
    return 0;
}

int32_t storage_write(const int32_t id, FRAME_INFO_T *frame_info)
{
    LOG_TRACE(logger, "storage write id %d, frame type %d, time %d.%d, stamp %"PRIu64", size %d", id, frame_info->type,
                         frame_info->frame_time.seconds, frame_info->frame_time.nseconds, frame_info->stamp, frame_info->size);
    return store_client_center->WriteFrame(id, frame_info);
}

int32_t storage_list_record_fragments(const int32_t id, const UTIME_T *start, const UTIME_T *end, 
        FRAGMENT_INFO_T **frag_info, uint32_t *count)
{
    LOG_INFO(logger, "list record fragments, id %d, start %d.%d, end %d.%d", id, start->seconds, start->nseconds, end->seconds, end->nseconds);

    int32_t ret;
    UTime start_time(start->seconds, start->nseconds);
    UTime end_time(end->seconds, end->nseconds);
    deque<FRAGMENT_INFO_T> frag_info_queue;

    ret = store_client_center->ListRecordFragments(id, start_time, end_time, frag_info_queue);
    if (ret != 0)
    {
        LOG_WARN(logger, "list record fragments error, ret %d, id %d, start %d.%d, end %d.%d", 
                ret, id, start->seconds, start->nseconds, end->seconds, end->nseconds);
        return ret;
    }
    
    *count = frag_info_queue.size();
    if (*count == 0)
    {
        LOG_INFO(logger, "list record fragments, count is 0");
        *frag_info = NULL;
        return 0;
    }

    FRAGMENT_INFO_T *frag_info_buffer = new FRAGMENT_INFO_T[*count];
    assert(frag_info_buffer != NULL);

    for (int i = 0; i < (int)*count; i++)
    {
        frag_info_buffer[i] = frag_info_queue.front();
        frag_info_queue.pop_front();
    }

    *frag_info = frag_info_buffer;

    return 0;
}

int32_t storage_free_record_fragments(FRAGMENT_INFO_T *frag_info, uint32_t count)
{
    LOG_INFO(logger, "storage free record fragments memory, frag info is %p, count is %d", frag_info, count);
    delete frag_info;
    frag_info = NULL;

    return 0;
}

int32_t storage_seek(const int32_t id, const UTIME_T *stamp)
{
    LOG_INFO(logger, "storage seek %d, stamp is %d.%d", id, stamp->seconds, stamp->nseconds);

    UTime time_stamp(stamp->seconds, stamp->nseconds);
    int32_t ret = store_client_center->SeekRead(id, time_stamp);
    if (ret != 0)
    {
        LOG_WARN(logger, "storage seek %d error, stamp is %d.%d, ret %d", id, stamp->seconds, stamp->nseconds, ret);
        return ret;
    }

    return 0;
}

int32_t storage_read(const int32_t id, FRAME_INFO_T *frame_info)
{
    LOG_DEBUG(logger, "storage read id %d", id);

    int32_t ret;
    ret = store_client_center->ReadFrame(id, frame_info);
    if (ret != 0)
    {
        LOG_WARN(logger, "storage read error, id %d, ret %d", id, ret);
        return ret; 
    }

    return 0;
}

void storage_close(const int32_t id)
{
    int32_t ret;
    int flag;

    LOG_INFO(logger, "storage close id %d", id);
    
    ret = id_center->GetFlag(id, flag);
    if (ret != 0)
    {
        LOG_WARN(logger, "get flag error, id %d, ret %d", id, ret);
        return;
    }

    id_center->ReleaseId(id);
    ret = store_client_center->Close(id, flag);
    if (ret != 0)
    {
        LOG_WARN(logger, "close id error, id %d, flag %d, ret %d", id, flag, ret);
    }

    return;
}

void storage_shutdown()
{
    LOG_INFO(logger, "storage shutdown");

    id_center->Shutdown();
    delete id_center;
    id_center = NULL;

    record_recycle->Shutdown();
    delete record_recycle;
    record_recycle = NULL;

    store_client_center->Shutdown();
    delete store_client_center;
    store_client_center = NULL;

    free_file_table->Shutdown();
    delete free_file_table;
    free_file_table = NULL;

    index_file_manager->Shutdown();
    delete index_file_manager;
    index_file_manager = NULL;

    dog_client->Shutdown();
    delete dog_client;
    dog_client = NULL;

    delete logger;
    logger = NULL;
    
    return;
}

