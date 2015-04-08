#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "index_file.h"
#include "record_file.h"
#include "../util/coding.h"
#include "../util/crc32c.h"
#include "storage.h"
#include "store_client.h"

using namespace util;

namespace storage
{

IndexFile::IndexFile(Logger *logger, string base_name)
: logger_(logger), mutex_("IndexFile::Locker"), base_name_(base_name)
{
    int ret = 0;

    string file_path;
    file_path = base_name_ + "index";
    index_file_ = fopen(file_path.c_str(), "rb+");
    if (index_file_ == NULL)
    {
        LOG_FATAL(logger_, "fopen error, base name [%s], errno msg is %s", file_path.c_str(), strerror(errno));
        assert(index_file_ != NULL);
    }

    /* read file counts */
    string file_count_path;
    FILE *file_count_handle = NULL;
    char file_count_str[32] = {0};
    file_count_path = base_name + "file_count";
    file_count_handle = fopen(file_count_path.c_str(), "r");
    assert(file_count_handle != NULL);
    ret = fread(file_count_str, 1, 32, file_count_handle);
    assert(ret < 32);
    fclose(file_count_handle);

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
    size_t ret;
    uint32_t record_file_section_size = 0;
    uint32_t record_file_info_length = 0;
    struct RecordFileInfo *record_file_info_buffer = NULL;

    record_file_info_length = sizeof(struct RecordFileInfo);
    record_file_section_size = file_counts_ * record_file_info_length;
    record_file_info_buffer = (struct RecordFileInfo *)malloc(record_file_section_size);
    assert(record_file_info_buffer != NULL);

    fseek(index_file_, 0, SEEK_SET);
    ret = fread((void*)record_file_info_buffer, record_file_info_length, file_counts_, index_file_);
    if (ret != file_counts_)
    {
        LOG_FATAL(logger_, "fread error, ret is %d, errno msg is [%s]", ret, strerror(errno));
        assert(ret == file_counts_);
    }

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
    int ret = 0;

    Mutex::Locker lock(mutex_);
    ret = fseek(index_file_, offset, SEEK_SET);
    assert(ret != -1);
    ret = fwrite(buffer, 1, length, index_file_);
    if (ret != (int)length)
    {
        LOG_FATAL(logger_, "fwrite error, index file %sindex, ret %d, length %u, errno msg is [%s]", 
                                base_name_.c_str(), ret, length, strerror(errno));
        assert(ret == (int)length);
    }
    fflush(index_file_);
    LOG_DEBUG(logger_, "index file write ok, base name %sindex, offset is %u, length is %u", base_name_.c_str(), offset, length);

    return 0;
}

int32_t IndexFile::Read(char *buffer, uint32_t length, uint32_t offset)
{
    LOG_DEBUG(logger_, "index file read, base name %s, length is %u, offset is %u", base_name_.c_str(), length, offset);
    int ret;

    Mutex::Locker lock(mutex_);
    ret = fseek(index_file_, offset, SEEK_SET);
    assert(ret != -1);

    ret = fread(buffer, 1, length, index_file_);
    if (ret != (int)length)
    {
        LOG_ERROR(logger_, "fread error, base name %sindex, ret %d, length %u, errno msg is [%s]", 
                                base_name_.c_str(), ret, length, strerror(errno));
        assert(ret == (int)length);
    }

    LOG_DEBUG(logger_, "index file read ok , length is %d, offset is %d", length, offset);

    return 0;
}

int32_t IndexFile::Shutdown()
{
    LOG_INFO(logger_, "shutdown");

    Mutex::Locker lock(mutex_);
    fclose(index_file_);

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
            FILE *fp = NULL;
            fp = fopen(temp_file_count_file.c_str(), "r");
            if (fp == NULL)
            {
                continue;
            }
            else
            {
                fclose(fp);
                fp = NULL;
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

