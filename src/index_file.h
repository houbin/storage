#ifndef STORAGE_INDEX_FILE_H_
#define STORAGE_INDEX_FILE_H_
#include <string.h>
#include <stdio.h>
#include "../util/logger.h"
#include "stream_transfer_client_manager.h"
#include "free_file_table.h"
#include "store_client_center.h"

using namespace std;

#pragma pack(1)
/* 这里定义的是硬盘上index file中的组织方式 */

/* 录像文件信息 */
struct RecordFileInfo
{
    uint32_t length;
    uint32_t crc;
    char stream_info[64];
    bool locked;
    char state_;
    uint16_t record_fragment_counts;
    UTime start_time;
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
};
#pragma pack()

class IndexFile
{
private:
    Logger *logger_;

    string base_name_;
    FILE *index_file_;
    uint32_t file_counts_;

public:
    IndexFile(Logger *logger, string base_name);

    char *GetBaseName();

    int32_t AnalyzeAllEntry(StreamTransferClientManager *client_transfer_manager, FreeFileTable *free_file_table);
    int32_t AnalyzeOneEntry(RecordFile *record_file);

    int32_t Write(uint32_t offset, char *buffer, uint32_t length);

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

    int32_t AddRecordFileEndEvent()
};

#endif
