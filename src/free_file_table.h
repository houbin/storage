#ifndef STORAGE_FREE_FILE_TABLE_H_
#define STORAGE_FREE_FILE_TABLE_H_

#include <set>
#include <deque>
#include "../util/cond.h"
#include "../util/mutex.h"
#include "../util/cond.h"
#include "../util/logger.h"
#include "../util/context.h"
#include "../util/timer.h"
#include "record_file.h"

using namespace std;
using namespace util;

namespace storage
{

struct DiskInfo
{
    set<string> writing_streams;
    deque<RecordFile*> free_file_queue;
};

class FreeFileTable
{
private:
    Logger *logger_;

    Mutex mutex_;
    Cond cond_;
    map<string, DiskInfo*> disk_free_file_info_; /* disk base name -> disk info, and disk base name == record file base name */
    map<string, string> stream_to_disk_map_; /* stream info -> disk base name */

    uint32_t CountRecordFiles();
    int32_t TryRecycle();

public:
    FreeFileTable(Logger *logger);

    int32_t AddDisk(string disk_name);

    int32_t Put(RecordFile *record_file);
    int32_t Get(string stream_info, RecordFile **record_file);

    int32_t GetNewDiskFreeFile(string stream_info, RecordFile **record_file);
    int32_t UpdateDiskWritingStream(string stream_info, RecordFile *record_file);

    int32_t CloseStream(string stream_info);
    int32_t Shutdown();

    void Dump();
};

}

#endif

