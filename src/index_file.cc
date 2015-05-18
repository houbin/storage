#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "index_file.h"
#include "record_file.h"
#include "../util/coding.h"
#include "../util/crc32c.h"
#include "storage.h"
#include "store_client.h"
#include "libaio_wrap.h"

using namespace util;

namespace storage
{

IndexFile::IndexFile(Logger *logger, string base_name)
: logger_(logger), mutex_("IndexFile::Locker"), base_name_(base_name), write_aio_ctx_(0), read_aio_ctx_(0)
{
    int ret = 0;
    ret = io_setup(8, &write_aio_ctx_);
    assert(ret == 0);

    ret = io_setup(8, &read_aio_ctx_);
    assert(ret == 0);

    string file_path;
    file_path = base_name_ + "index";
    fd_ = open(file_path.c_str(), O_RDWR);
    if (fd_ < 0)
    {
        LOG_FATAL(logger_, "open error, base name [%s], errno msg is %s", file_path.c_str(), strerror(errno));
        assert(fd_ >= 0);
    }

    /* read file counts */
    string file_count_path;
    int file_count_fd = 0;
    char file_count_str[32] = {0};
    io_context_t ctx;

    ctx = 0;
    ret = io_setup(128, &ctx);
    assert(ret == 0);

    file_count_path = base_name + "file_count";
    file_count_fd = open(file_count_path.c_str(), O_RDONLY);
    assert(file_count_fd > 0);

    int32_t r = 0;
    r = libaio_single_read(ctx, file_count_fd, file_count_str, 31, 0);
    if (r <= 0)
    {
        LOG_FATAL(logger_, "libaio read file_count error, r %d, dir %s", r, base_name_.c_str());
        bad_disk_map->AddBadDisk(base_name_);

        io_destroy(ctx);
        close(file_count_fd);
        return;
    }
    assert(r > 0);

    io_destroy(ctx);
    close(file_count_fd);

    file_counts_ = atoi(file_count_str);
}

string IndexFile::GetBaseName()
{
    return base_name_;
}

uint32_t IndexFile::GetFileCounts()
{
    return file_counts_;
}

int32_t IndexFile::AnalyzeAllEntry()
{
    int32_t ret;
    uint32_t record_file_section_size = 0;
    uint32_t record_file_info_length = 0;
    struct RecordFileInfo *record_file_info_buffer = NULL;

    record_file_info_length = sizeof(struct RecordFileInfo);
    record_file_section_size = file_counts_ * record_file_info_length;
    record_file_info_buffer = (struct RecordFileInfo *)malloc(record_file_section_size);
    assert(record_file_info_buffer != NULL);

    ret = libaio_single_read(read_aio_ctx_, fd_, (char *)record_file_info_buffer, record_file_section_size, 0);
    if (ret < 0)
    {
        LOG_FATAL(logger_, "libaio read index error, ret %d, dir %s", ret, base_name_.c_str());

        bad_disk_map->AddBadDisk(base_name_);
        safe_free(record_file_info_buffer);
        return -ERR_BAD_DISK;
    }
    assert(ret == (int32_t)record_file_section_size);

    uint32_t i = 0;
    for (i = 0; i < file_counts_; i++)
    {
        char *temp = (char *)(&(record_file_info_buffer[i]));

        RecordFile *record_file = new RecordFile(logger_, base_name_, i);
        assert(record_file != NULL);

        uint32_t length = DecodeFixed32(temp);
        temp += 4;

        if (length == 0)
        {
            /* not used */
            free_file_table->Put(record_file);
            continue;
        }

        uint32_t expected_crc = DecodeFixed32(temp);
        temp += 4;

        uint32_t actual_crc = crc32c::Value(temp, length);
        assert(expected_crc == actual_crc);

        ret = record_file->DecodeRecordFileInfoIndex(temp, length);
        assert(ret == 0);

        string stream_info(record_file->stream_info_);
        StoreClient *store_client = NULL;
        int32_t ret = store_client_center->FindStoreClient(stream_info, &store_client);
        if (ret != 0)
        {
            ret = store_client_center->AddStoreClient(stream_info, &store_client);
        }
        assert(ret == 0);
        ret = store_client->PutRecordFile(record_file->start_time_, record_file);
        assert(ret == 0);
    }

    safe_free(record_file_info_buffer);
    LOG_INFO(logger_, "analyze all entry end");
    return 0;
}

int32_t IndexFile::Write(uint32_t offset, char *buffer, uint32_t length)
{
    LOG_DEBUG(logger_, "index file write, base name %sindex, offset is %u, length is %u", base_name_.c_str(), offset, length);
    int32_t ret = 0;

    mutex_.Lock();
    ret = libaio_single_write(write_aio_ctx_, fd_, buffer, length, offset);
    if (ret != (int32_t)length)
    {
        mutex_.Unlock();
        bad_disk_map->AddBadDisk(base_name_);
        return -ERR_BAD_DISK;
    }

    LOG_DEBUG(logger_, "index file write ok, base name %sindex, offset is %u, length is %u", base_name_.c_str(), offset, length);

    mutex_.Unlock();

    return 0;
}

int32_t IndexFile::Read(char *buffer, uint32_t length, uint32_t offset)
{
    LOG_DEBUG(logger_, "index file read, base name %s, length is %u, offset is %u", base_name_.c_str(), length, offset);
    int32_t ret = 0;

    mutex_.Lock();
    ret = libaio_single_read(read_aio_ctx_, fd_, buffer, length, offset);
    if (ret < 0)
    {
        LOG_WARN(logger_, "libaio read error, ret %d, dir %s", ret, base_name_.c_str());
        mutex_.Unlock();
        bad_disk_map->AddBadDisk(base_name_);
        return -ERR_BAD_DISK;
    }

    assert(ret == (int)length);
    mutex_.Unlock();

    LOG_DEBUG(logger_, "index file read, base name %s, length is %u, offset is %u", base_name_.c_str(), length, offset);

    return 0;
}

int32_t IndexFile::Shutdown()
{
    LOG_INFO(logger_, "shutdown");

    Mutex::Locker lock(mutex_);
    io_destroy(write_aio_ctx_);
    io_destroy(read_aio_ctx_);
    close(fd_);

    return 0;
}

IndexFileManager::IndexFileManager(Logger *logger)
: logger_(logger), mutex_("IndexFileManager::Lock")
{

}

int32_t IndexFileManager::Init()
{
    ScanAllIndexFile();
    AnalyzeAllIndexFile();
    return 0;
}

int32_t IndexFileManager::ScanAllIndexFile()
{
    DIR *dp = NULL;
    struct dirent *entry = NULL;
    struct stat statbuf;

    dp = opendir("/jovision/mnt/");
    assert(dp != NULL);

    chdir("/jovision/mnt/");
    while((entry = readdir(dp)) != NULL)
    {
        lstat(entry->d_name, &statbuf);
        if (S_ISDIR(statbuf.st_mode))
        {
            if ((strcmp(".", entry->d_name) == 0) 
                || (strcmp("..", entry->d_name) == 0))
            {
                continue;
            }

            IndexFile *index_file = NULL;
            string base_name("/jovision/mnt/");
            base_name = base_name + entry->d_name + "/";

            /* test if dir is legal */
            string temp_file_count_file = base_name + "file_count";
            int fd = 0;
            fd = open(temp_file_count_file.c_str(), O_RDONLY);
            if (fd < 0)
            {
                LOG_INFO(logger, "open error, dir %s", temp_file_count_file.c_str());
                continue;
            }
            else
            {
                close(fd);
                fd = 0;
            }

            index_file = new IndexFile(logger_, base_name);
            assert(index_file != NULL);

            index_file_map_.insert(make_pair(base_name, index_file));

            LOG_INFO(logger_, "insert index file ok, base name %s", base_name.c_str());
        }
    }

    closedir(dp);
    LOG_INFO(logger_, "scan all index file end");

    return 0;
}

int32_t IndexFileManager::AnalyzeAllIndexFile()
{
    LOG_INFO(logger_, "analyze all index file");

    map<string, IndexFile*>::iterator iter = index_file_map_.begin();
    for(; iter != index_file_map_.end(); iter++)
    {
        IndexFile *index_file = iter->second;
        string disk_name = index_file->GetBaseName();

        free_file_table->AddDisk(disk_name);
        index_file->AnalyzeAllEntry();
    }

    LOG_INFO(logger_, "analyze all index file end");

    return 0;
}

int32_t IndexFileManager::Find(string base_name, IndexFile **index_file)
{
    LOG_DEBUG(logger_, "find index file, base name %s", base_name.c_str());

    Mutex::Locker lock(mutex_);
    map<string, IndexFile*>::iterator iter = index_file_map_.find(base_name);
    if (iter == index_file_map_.end())
    {
        LOG_WARN(logger_, "find index file failed, base name %s", base_name.c_str());
        return -ERR_ITEM_NOT_FOUND;
    }

    *index_file = iter->second;

    return 0;
}

int32_t IndexFileManager::Shutdown()
{
    LOG_INFO(logger_, "shutdown");
    
    Mutex::Locker lock(mutex_);
    while (!index_file_map_.empty())
    {
        map<string, IndexFile*>::iterator iter = index_file_map_.begin();
        IndexFile *index_file = iter->second;
        index_file_map_.erase(iter);

        index_file->Shutdown();
        delete index_file;
    }

    LOG_INFO(logger_, "shutdown end");

    return 0;
}

int32_t IndexFileManager::GetFileNumbers()
{
    int32_t sum = 0;
    Mutex::Locker lock(mutex_);
    
    map<string, IndexFile*>::iterator iter = index_file_map_.begin();
    for (; iter != index_file_map_.end(); iter++)
    {
        IndexFile *index_file = iter->second;
        sum += index_file->GetFileCounts();
    }

    return sum;
}

void IndexFileManager::Dump()
{
    LOG_INFO(logger, "******** index file manager record file sum %d ********", GetFileNumbers());
    return;
}

}

