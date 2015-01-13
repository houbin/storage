#ifndef STORAGE_RECORD_FILE_
#define STORAGE_RECORD_FILE_

#include <string>

namespace storage
{
#define BUFFER_COUNT_LIMIT 256
#define RECORD_FILE_SIZE (BUFFER_COUNT_LIMIT * BUFFER_LENGTH)

using namespace std;
class RecordFile
{
public: 
    string base_name_;
    uint32_t number_; // 文件编号
    uint32_t stream_id_;
    bool locked_;
    bool used_;

    uint16_t record_fragment_count_;
    UTime start_time_;
    UTime end_time_; 
    UTime i_frame_start_time_;
    UTime i_frame_end_time_;

    uint32_t record_offset_;

    RecordFile(string base_name, uint32_t number);

    /* 将该录像文件对应的索引段清零 */
    int32_t ClearIndex();
};


}
#endif
