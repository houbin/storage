#ifndef STORAGE_INDEX_FILE_H_
#define STORAGE_INDEX_FILE_H_
#include <string.h>
#include <stdio.h>
#include "../util/logger.h"
#include "stream_transfer_client_manager.h"

using namespace std;

struct RecordFileInfo
{
    uint32_t length;
    uint32_t crc;
    uint32_t stream_id;
    bool used;
    bool locked;
    uint16_t record_fragment_counts;
    UTime start_time;
    UTime end_time;
};

struct RecordFragmentInfo
{
    uint32_t length;
    uint32_t crc;
    UTime start_time;
    UTime end_time;
    char padding[8];
};

class IndexFile
{
private:
    Logger *logger_;

    string base_name_;
    FILE *index_file_;
    uint32_t file_counts_;

public:
    IndexFile(Logger *logger, string base_name);
    ~IndexFile();

    int32_t Init(StreamTransferClientManager *client_transfer_manager);
};

class IndexFileManager
{
private:
    Logger *logger_;

    map<string, IndexFile*> index_file_map_;

    bool shutdown_;
    
    StreamTransferClientManager *transfer_client_manager_;

public:
    IndexFileManager(Logger *logger, StreamTransferClientManager *transfer_client_manager);

    int32_t Init();
    int32_t Find(string base_name, IndexFile *index_file);
    int32_t Shutdown();
};

#endif
