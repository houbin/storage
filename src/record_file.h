#ifndef STORAGE_RECORD_FILE_
#define STORAGE_RECORD_FILE_

#include <string>
#include "stream_map.h"

namespace storage
{
#define BUFFER_COUNT_LIMIT 256
#define RECORD_FILE_SIZE (BUFFER_COUNT_LIMIT * BUFFER_LENGTH)

using namespace std;
class RecordFile
{
public:

private:
    string mount_point_;
    uint16_t number_; // 文件编号
    bool locked; // 文件是否被用户锁定

    UTime start_time_;
    UTime end_time_; 
};

}
#endif
