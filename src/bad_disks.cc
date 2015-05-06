#include "bad_disks.h"

namespace storage
{

void BadDiskMap::Init()
{
    return;
}

bool BadDiskMap::CheckIfBadDisk(string disk)
{
    RWLock::RDLocker lock(rwlock_);

    set<string>::iterator iter = bad_disks_.find(disk);
    if (iter == bad_disks_.end())
    {
        return false;
    }

    return true;
}

int32_t BadDiskMap::AddBadDisk(string bad_disk)
{
    RWLock::WRLocker lock(rwlock_);

    set<string>::iterator iter = bad_disks_.find(bad_disk);
    if (iter != bad_disks_.end())
    {
        LOG_WARN(logger_, "already have this bad disk %s", bad_disk.c_str());
        return 0;
    }

    bad_disks_.insert(bad_disk);

    LOG_WARN(logger_, "add bad disk %s", bad_disk.c_str());

    return 0;
}

void BadDiskMap::Shutdown()
{
    return;
}

}
