#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include "index_file.h"
#include "record_file.h"
#include "../util/coding.h"

namespace storage
{

IndexFile::IndexFile(Logger *logger, string base_name)
: logger_(logger), base_name_(base_name)
{
    int ret = 0;
    string file_path;
    FILE *file_count_handle = NULL;

    file_path = base_name + "index";
    index_file_ = fopen(file_path.c_str(), "r");
    assert(index_file_ != NULL);

    /* read file counts */
    string file_count_path;
    char file_count_str[32] = {0};
    file_count_path = base_name + "file_count";
    file_count_handle = fopen(file_count_path.c_str(), "r");
    assert(file_count_handle != NULL);
    ret = fread(file_count_str, 1, 32, file_count_handle);
    assert(ret < 32);
    fclose(file_count_handle);

    file_counts_ = atoi(file_count_str);

    Log(logger, "%s file count %d", base_name_.c_str(), file_counts_);
}

int32_t IndexFile::AnalyzeOneEntry(char *buffer, RecordFile *record_file)
{
    char *temp = NULL;

    assert(buffer != NULL);
    assert(record_file != NULL);

    Log(logger_, "analyze one entry");

    temp = buffer;
    record_file->stream_id_ = DecodeFixed32(temp);
    temp += 4;
    record_file->locked_ = temp;
    temp += 1;
    record_file->used_ = temp;
    temp += 1;
    record_file->record_fragment_counts_ = temp | ((temp + 1) << 8);
    temp += 2;
    record_file->start_time.tv_sec = DecodeFixed32(temp);
    temp += 4;
    record_file->start_time.tv_nsec = DecodeFixed32(temp);
    temp += 4;
    record_file->i_frame_start_time_.tv_sec = DecodeFixed32(temp);
    temp += 4;
    record_file->i_frame_end_time_.tv_nsec = DecodeFixed32(temp);
    temp += 4;
    record_file->record_offset_ = DecodeFixed32(temp);

    return 0;
}

int32_t IndexFile::AnalyzeAll(StreamTransferClientManager *transfer_client_manager, FreeFileTable *free_file_table)
{
    size_t ret;
    uint32_t record_file_section_size = 0;
    uint32_t record_file_info_length = 0;
    struct RecordFileInfo *record_file_info = NULL;

    assert(transfer_client_manager != NULL);
    assert(free_file_table != NULL);

    record_file_info_length = sizeof(struct RecordFileInfo);
    record_file_section_size = file_counts_ * record_file_info_length;
    record_file_info_buffer = (struct RecordFileInfo *)malloc(record_file_section_size);
    assert(record_file_info_buffer != NULL);

    ret = fread((void*)record_file_info_buffer, record_file_info_length, file_counts, index_file_);
    assert(ret == file_counts);

    int i = 0;
    for (i = 0; i < file_counts; i++)
    {
        char *temp = (char *)(record_file_info_buffer[i]);

        RecordFile *record_file = new RecordFile(base_name_, i);
        assert(record_file != NULL);

        uint32_t length = DecodeFixed32(temp);
        temp += 4;

        if (length == 0)
        {
            /* not used */
            free_file_table.Put(record_file);
            continue;
        }

        uint32_t expected_crc = DecodeFixed32(temp);
        temp += 4;
        uint32_t actual_crc = crc32c::Value(temp, length);
        assert(expected_crc == actual_crc);
        temp += 4;
        AnalyzeOneEntry(temp, record_file);

        uint32_t stream_id = record_file->stream_id_;
        StreamTransferClient *transfer_client;
        int32_t ret = transfer_client_manager->Find(stream_id, &transfer_client);
        assert(ret == 0);
        ret = transfer_client->Insert(record_file);
        assert(ret == 0);
    }

    Log(logger_, "analyze all entry end");
    return 0;
}

IndexFile::~IndexFile()
{
    fclose(index_file_);
}

IndexFileManager::IndexFileManager(Logger *logger, StreamTransferClientManager *client_transfer_manager, FreeFileTable *free_file_table)
: logger_(logger), shutdown_(false), transfer_client_manager_(transfer_client_manager), free_file_table_(free_file_table)
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

    Log(logger_, "scan all index file");

    dp = opendir("/jovision");
    assert(dp != NULL);

    chdir("/jovision");
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
            string base_name("/jovision/");
            base_name = base_name + entry->d_name + "/";
            index_file = new IndexFile(logger_, base_name);
            assert(index_file != NULL);

            index_file_map_.insert(base_name, index_file);

            Log(logger_, "insert index file %s", base_name);
        }
    }

    closedir(dp);
    Log(logger_, "scan all index file end");

    return 0;
}

int32_t IndexFileManager::AnalyzeAllIndexFile()
{
    Log(logger_, "analyze all index file");

    map<string, IndexFile*>::iterator iter = index_file_map_.begin();
    for(iter; iter != index_file_map_.end(); iter++)
    {
        IndexFile *index_file = iter->second;

        index_file->Analyze(transfer_client_manager_, free_file_table_);
    }

    Log(logger_, "analyze all index file end");

    return 0;
}

int32_t IndexFileManager::Find(string base_name, IndexFile *index_file)
{
    Log(logger_, "find index file %s", base_name.c_str());
    if (shutdown_)
    {
        return -1;
    }

    map<string, IndexFile*>::iterator iter = index_file_map_.find(base_name);
    if (iter == index_file_map_.end())
    {
        Log(logger_, "not found");
        return -1;
    }

    index_file = iter->second;

    return 0;
}

int32_t IndexFileManager::Shutdown()
{
    shutdown_ = true;
    Log(logger_, "shutdown");
    
    map<string, IndexFile*>::iterator iter = index_file_map_.begin();
    for(iter; iter != index_file_map_.end(); iter++)
    {
        IndexFile *index_file;
        index_file = iter->second;
        
        index_file_map_.erase(iter);
        delete index_file;
    }

    return 0;
}


}
