#ifndef STORAGE_H_
#define STORAGE_H_

#include "../util/logger.h"

namespace storage
{

class IdManager
{
private:
    Logger *logger;
    Mutex mutex_;

    uint32_t next_id_;

    map<uint32_t, 

public:
    IdManager(Logger *logger);

    /* stream info的长度统一为64个字节 */
    int32_t ApplyForId(char *stream_info, uint32_t size, uint32_t *id);
    int32_t ReleaseId(uint32_t id);

    int32_t GetStreamInfoFromId(uint32_t id, char *stream_info);
};

class Storage
{
private:
    Logger *logger_;
    
    IdMap id_mapper_;

public:
    Storage(Logger *logger, )
};

}

#endif
