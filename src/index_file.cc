#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include "index_file.h"
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

int32_t IndexFile::Init(StreamTransferClientManager *transfer_client_manager)
{
    size_t ret;
    uint32_t record_file_section_size = 0;
    struct RecordFileInfo *record_file_info = NULL;

    assert(transfer_client_manager != NULL);

    record_file_section_size = file_counts_ * sizeof(struct RecordFileInfo);
    record_file_info = (struct RecordFileInfo *)malloc(record_file_section_size);
    assert(record_file_info != NULL);

    ret = fread((void*)record_file_info, 32, file_counts, index_file_);
    assert(ret == file_counts);

    int i = 0;
    for (i = 0; i < file_counts; i++)
    {
        uint32_t length;
        uint32_t expected_crc;
        uint32_t actual_crc;
        
        char *temp = record_file_info[i];

        length = DecodeFixed32(temp);
        if (length == 0)
        {
            /* not used */
            continue;
        }
        expected_crc = DecodeFixed32(temp + 4);
        actual_crc = crc32c::Value(temp+8, length);
        assert(expected_crc == actual_crc);
        


    }



    
}

IndexFile::~IndexFile()
{
    fclose(index_file_);
}

IndexFileManager::IndexFileManager(Logger *logger, StreamTransferClientManager *client_transfer_manager)
: logger_(logger), shutdown_(false), transfer_client_manager_(transfer_client_manager)
{

}

int32_t IndexFileManager::Init()
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;

    Log(logger_, "init");

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

    map<string, IndexFile*>::iterator iter = index_file_map_.begin();
    for(iter; iter != index_file_map_.end(); iter++)
    {
        IndexFile *index_file = iter->second;

        index_file->Init(transfer_client_manager_);
    }


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
