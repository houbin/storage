#include "stream_id_manager.h"

namespace storage
{

int32_t StreamIdManager::ApplyForFreeId()
{
    int free_id;
    Mutex::Locker locker(lock_);

    free_id = bitmap_.get_free_bit();
    if (free_id < 0)
    {
        Log(logger_, "get free bit error");
        return -1;
    }

    bitmap_.set(free_id);

    return free_id;
}

int32_t StreamIdManager::ReleaseId(uint32_t id)
{
    Mutex::Locker locker(lock_);

    bitmap_.clear(id);

    return 0;
}

}
