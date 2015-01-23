#ifndef STORAGE_ID_MANAGER_H_
#define STORAGE_ID_MANAGER_H_

#include <map>
#include "../util/logger.h"
#include "../util/mutex.h"

namespace storage
{

class IdCenter
{
private:
    Logger *logger_;
    Mutex mutex_;

    uint32_t next_id_;

    map<uint32_t, string> id_map_;
    map<string, uint32_t> write_key_info_;

public:
    IdCenter(Logger *logger);

    int32_t ApplyForId(string key_info, int flags, uint32_t *id);
    int32_t ReleaseId(uint32_t id);

    int32_t GetStreamInfoFromId(uint32_t id, string &key_info);
};

}
#endif
