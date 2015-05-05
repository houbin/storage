#include "bad_disks.h"

namespace storage
{

bool BadDiskMap::CheckIfBadDisk(string disk)
{
    Mutex::Locker lock(mutex_);

    set<string>::iterator iter = bad_disks_.find(disk);
    if (iter == bad_disks_.end())
    {
        return false;
    }

    return true;
}

int32_t BadDiskMap::AddBadDisk(string bad_disk)
{
    Mutex::Locker lock(mutex_);

    bad_disks_.insert(bad_disk);

    LOG_WARN(logger_, "add bad disk %s", bad_disk.c_str());

    return 0;
}

}
