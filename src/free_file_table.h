#ifndef STORAGE_FREE_FILE_TABLE_H_
#define STORAGE_FREE_FILE_TABLE_H_

#include <deque>
#include "../util/cond.h"
#include "../util/mutex.h"
#include "../util/cond.h"
#include "../util/logger.h"
#include "../util/context.h"
#include "../util/timer.h"
#include "record_file.h"
#include "stream_transfer_client_manager.h"

using namespace std;
using namespace util;

namespace storage
{

typedef struct st_disk_info
{
    set<string> writing_streams;
    deque<RecordFile*> free_file_queue;
}DiskInfo;

class FreeFileTable
{
private:
    Logger *logger_;

    Mutex mutex_;
    Cond cond_;
    map<string, DiskInfo*> disk_free_file_info_; /* disk base name -> disk info, and disk base name == record file base name */
    map<string, string> stream_to_disk_map_; /* stream info -> disk map */

public:
    FreeFileTable(Logger *logger);

    int32_t Put(RecordFile *record_file);
    int32_t Get(string stream_info, RecrodFile **record_file);

    int32_t GetNewDiskFreeFile(string stream_info, RecordFile **record_file);

    int32_t Shutdown();
};

}

#endif

