#include "stream_id_manager.h"

namespace storage
{

int32_t StreamIdManager::ApplyForFreeId(uint64_t *stream_id)
{
    Mutex::Locker locker(lock_);

    free_id = bitmap_.get_free_bit();
    if (free_id < 0)
    {
        Log(logger_, "get free bit error");
        return -1;
    }

    bitmap_.set(free_id);
    *stream_id = free_id;

    return 0;
}

int32_t StreamIdManager::ReleaseId(uint32_t id)
{
    Mutex::Locker locker(lock_);

    bitmap_.clear(id);

    return 0;
}

}
