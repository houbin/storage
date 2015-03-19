#ifndef STORAGE_ID_MANAGER_H_
#define STORAGE_ID_MANAGER_H_

#include <map>
#include "../util/logger.h"
#include "../util/mutex.h"

using namespace std;
using namespace util;

namespace storage
{

class IdCenter
{
private:
    Logger *logger_;
    Mutex mutex_;

    int32_t next_id_;

    map<int32_t, string> id_map_;
    map<string, int32_t> write_key_info_;
    map<int32_t, string> read_id_map_;

public:
    IdCenter(Logger *logger);

    int32_t ApplyForId(string key_info, int flags, int32_t *id);
    int32_t GetStreamInfoFromId(int32_t id, string &key_info);
    int32_t ReleaseId(int32_t id);

    int32_t GetFlag(int32_t id, int &flag);
    void Shutdown();
};

}
#endif

