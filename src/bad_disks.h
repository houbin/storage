#ifndef STORAGE_BAD_DISKS_H_
#define STORAGE_BAD_DISKS_H_

#include <string.h>
#include <set>
#include "../util/logger.h"
#include "../util/rwlock.h"

using namespace std;
using namespace util;

namespace storage
{

class BadDiskMap
{
private:
    Logger *logger_;
    RWLock rwlock_;
    set<string> bad_disks_;

public:
    BadDiskMap(Logger *logger) : logger_(logger), rwlock_("UnUsableDiskMap::rwlock")
    {
    
    }

    void Init();

    bool CheckIfBadDisk(string disk);
    int32_t AddBadDisk(string bad_disk);

    void Shutdown();
};

}

#endif

