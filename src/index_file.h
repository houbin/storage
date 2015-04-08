#ifndef STORAGE_INDEX_FILE_H_
#define STORAGE_INDEX_FILE_H_

#include <string.h>
#include <stdio.h>
#include "../util/logger.h"
#include "free_file_table.h"
#include "store_client_center.h"
#include "record_file.h"

using namespace std;

namespace storage
{

class IndexFile
{
private:
    Logger *logger_;
    Mutex mutex_;

    string base_name_;
    FILE *index_file_;
    uint32_t file_counts_;

public:
    IndexFile(Logger *logger, string base_name);

    string GetBaseName();

    uint32_t GetFileCounts();
    int32_t AnalyzeAllEntry();

    int32_t Write(uint32_t offset, char *buffer, uint32_t length);
    int32_t Read(char *buffer, uint32_t length, uint32_t offset);
    int32_t Shutdown();
};

class IndexFileManager
{
private:
    Logger *logger_;

    Mutex mutex_;
    map<string, IndexFile*> index_file_map_;

public:
    IndexFileManager(Logger *logger);

    int32_t ScanAllIndexFile();
    int32_t AnalyzeAllIndexFile();

    int32_t Init();
    int32_t Find(string base_name, IndexFile **index_file);
    int32_t Shutdown();

    int32_t GetFileNumbers();
    void Dump();
};

}

#endif

