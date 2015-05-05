#ifndef STORAGE_BAD_DISKS_H_
#define STORAGE_BAD_DISKS_H_

#include <string.h>
#include <set>
#include "../util/logger.h"

using namespace std;
using namespace util;

namespace storage
{

class BadDiskMap
{
private:
    Logger *logger_;
    Mutex mutex_;
    set<string> bad_disks_;

public:
    BadDiskMap(Logger *logger) : logger_(logger), mutex_("UnUsableDiskMap::mutex")
    {
    
    }

    bool CheckIfBadDisk(string disk);
    int32_t AddBadDisk(string bad_disk);
};

}

#endif

