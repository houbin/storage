#ifndef STORAGE_INDEX_FILE_H_
#define STORAGE_INDEX_FILE_H_
#include <string.h>
#include <stdio.h>
#include "../util/logger.h"
#include "stream_transfer_client_manager.h"
#include "free_file_table.h"

using namespace std;

#pragma pack(4)
/* 这里定义的是硬盘上index file中的组织方式 */

/* 录像文件信息 */
struct RecordFileInfo
{
    uint32_t length;
    uint32_t crc;
    uint32_t stream_id;
    bool locked;
    bool used;
    uint16_t record_fragment_counts;
    UTime start_time;
    uint32_t start_frame_offset_;

    UTime end_time;
    UTime i_frame_start_time;
    UTime i_frame_end_time;
    uint32_t record_offset;
    char padding[16];
}; 

/* 录像片段信息 */
struct RecordFragmentInfo
{
    uint32_t length;
    uint32_t crc;
    UTime start_time;
    UTime end_time;
    UTime i_frame_start_time;
    UTime i_frame_end_time;
}; // 32 bytes
#pragma pack()

struct IndexFileOp
{
    string index_file_base_name;
    uint32_t offset;
    uint32_t length;
    char *buffer;

    IndexFileOp(string name, uint32_t o, uint32_t l, char *buf)
    : index_file_base_name(name), offset(o), length(l), buffer(buf)
    {}
};

class IndexFile
{
private:
    Logger *logger_;

    string base_name_;
    FILE *index_file_;
    Mutex mutex_;
    Cond cond_;
    uint32_t file_counts_;

    deque<struct IndexFileOp*> op_queue_;
    bool stop_;

    struct WriteThread : public Thread
    {
        IndexFile *index_file;
        WriteThread(IndexFile *index_file): index_file_(index_file) {}
        void *entry()
        {
            index_file_->WriteEntry();
            return 0;
        }
        
    } write_thread_;

public:
    IndexFile(Logger *logger, string base_name);

    char *GetBaseName();

    int32_t AnalyzeAllEntry(StreamTransferClientManager *client_transfer_manager, FreeFileTable *free_file_table);
    int32_t AnalyzeOneEntry(RecordFile *record_file);

    int32_t EnqueueOp(struct IndexFileOp *index_file_op);
    int32_t DequeueOp(struct IndexFileOp **index_file_op);
    int32_t WriteEntry();

    int32_t DoOneOp(struct IndexFileOp *op);
    int32_t DoAllOps();
    int32_t Shutdown();
};

class IndexFileManager
{
private:
    Logger *logger_;
    Mutex mutex_;

    map<string, IndexFile*> index_file_map_;

    StreamTransferClientManager *transfer_client_manager_;
    FreeFileTable *free_file_table_;

    bool stop_;

public:
    IndexFileManager(Logger *logger, StreamTransferClientManager *transfer_client_manager, FreeFileTable *free_file_table);

    int32_t ScanAllIndexFile();
    int32_t AnalyzeAllIndexFile();

    int32_t Init();
    int32_t Find(string base_name, IndexFile **index_file);
    int32_t Shutdown();
};

#endif
