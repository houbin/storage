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

namespace storage
{

IndexFile::IndexFile(Logger *logger, string base_name)
: logger_(logger), mutex_("IndexFile::locker"), base_name_(base_name), stop_(false)
{
    int ret = 0;
    string file_count_path;
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

char *IndexFile::GetBaseName()
{
    return base_name_.c_str();
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
    record_file->start_frame_offset_ = DecodeFixed32(temp);
    temp += 4;
    record_file->i_frame_start_time_.tv_sec = DecodeFixed32(temp);
    temp += 4;
    record_file->i_frame_end_time_.tv_nsec = DecodeFixed32(temp);
    temp += 4;
    record_file->record_offset_ = DecodeFixed32(temp);

    return 0;
}

int32_t IndexFile::AnalyzeAllEntry(StreamTransferClientManager *transfer_client_manager, FreeFileTable *free_file_table)
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

    /* 启动写线程 */
    write_thread_.Create();

    Log(logger_, "analyze all entry end");
    return 0;
}

int32_t IndexFile::EnqueueOp(struct IndexFileOp *index_file_op)
{
    assert(index_file_op != NULL);
    Log(logger_, "enqueue op %p", index_file_op);

    Mutex::Locker lock(mutex_);
    if (stop_)
    {
        Log(logger_, "should not go here");
        delete index_file_op;
        return 0;
    }

    op_queue_.push_back(index_file_op);
    cond_.Signal();

    return 0;
}

int32_t IndexFile::DequeueOp(struct IndexFileOp **index_file_op)
{
    assert(index_file_op != NULL);
    Log(logger_, "dequeue op");

    deque<struct IndexFileOp*>::iterator iter = op_queue_.begin();
    *index_file_op = *iter;
    op_queue_.erase(iter);

    return 0;
}

int32_t IndexFile::WriteEntry()
{
    int32_t ret;

    Log(logger_, "index file write entry");
    mutex_.Lock();

    while(true)
    {
        while(!op_queue_.empty())
        {
            struct IndexFileOp *op;
            ret = DequeueOp(&op);
            assert(ret == 0);

            mutex_.Unlock();

            /* 处理该请操作 */
            DoOneOp(op);

            mutex_.Lock();
        }

        if (stop_)
        {
            break;
        }
        
        cond_.Wait(mutex_);
    }

    mutex_.Unlock();
    Log(logger_, "index file write entry end");

    return 0;
}

int32_t IndexFile::DoOneOp(struct IndexFileOp *op)
{
    int32_t ret;

    assert(op != NULL);
    assert(op->buffer != NULL);

    Log(logger_, "do on op, op is %p, base name is %s, offset is %d, length is %d",
        op, op->index_file_base_name, op->offset, op->length);

    ret = lseek(fd_, op->offset, SEEK_SET);
    assert(ret == op->offset);

    ret = write(fd_, op->buffer, op->length);
    assert(ret == op->length);

    delete op->buffer;
    delete op;

    return 0;
}

int32_t IndexFile::DoAllOps()
{
    Log(logger_, "do all ops");

    while (!op_queue_.empty())
    {
        struct IndexFileOp *op = op_queue_.front();
        assert(op != NULL);
        op_queue_.pop_front();

        DoOneOp(op);
    }

    return 0;
}

int32_t IndexFile::Shutdown()
{
    Log(logger_, "shutdown");

    mutex_.Lock();
    stop_ = true;
    cond_.Signal();
    mutex_.Unlock();

    write_thread_.Join();

    /* don't need to lock */
    DoAllOps();

    if (index_file_ != NULL);
    {
        fclose(index_file_);
        index_file_ = NULL;
    }

    return 0;
}

IndexFileManager::IndexFileManager(Logger *logger, StreamTransferClientManager *client_transfer_manager, FreeFileTable *free_file_table)
: logger_(logger), mutex_(IndexFileManager::Lock), shutdown_(false), transfer_client_manager_(transfer_client_manager), free_file_table_(free_file_table), stop_(false)
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

    Mutex::Locker lock(mutex_);
    map<string, IndexFile*>::iterator iter = index_file_map_.begin();
    for(iter; iter != index_file_map_.end(); iter++)
    {
        IndexFile *index_file = iter->second;
        index_file->AnalyzeAllEntry(transfer_client_manager_, free_file_table_);
    }

    Log(logger_, "analyze all index file end");

    return 0;
}

int32_t IndexFileManager::Find(string base_name, IndexFile **index_file)
{
    Log(logger_, "find index file %s", base_name.c_str());

    Mutex::Locker lock(mutex_);
    if (stop_)
    {
        Log(logger_, "stopped");
        return -1;
    }

    map<string, IndexFile*>::iterator iter = index_file_map_.find(base_name);
    if (iter == index_file_map_.end())
    {
        Log(logger_, "not found");
        return -1;
    }

    *index_file = iter->second;

    return 0;
}

int32_t IndexFileManager::Shutdown()
{
    Log(logger_, "shutdown");
    
    Mutex::Locker lock(mutex_);
    stop_ = true;
    while (!index_file_map_.empty())
    {
        map<string, IndexFile*>::iterator iter = index_file_map_.begin();
        IndexFile *index_file = iter->second;
        index_file_map_.erase(iter);

        Log(logger_, "shutdown index file %p, base_name is %s", index_file, index_file.GetBaseName());
        index_file->Shutdown();
        delete index_file;
    }

    Log(logger_, "shutdown end");

    return 0;
}

}
