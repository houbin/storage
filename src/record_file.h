#ifndef STORAGE_RECORD_FILE_
#define STORAGE_RECORD_FILE_

#include <string>
#include "../util/logger.h"
#include "../util/mutex.h"

namespace storage
{
using namespace std;

enum RecordFileState
{
    kCleared = 0,
    kReadOnly = 1,  // 若record file初始化后写数据之前或者已经写完，则处于read only状态
    kWriting = 2,     // record file有数据写入，则record file处于write状态
};

class RecordFile
{
public: 
    Logger *logger_;

    string base_name_;
    uint32_t number_; // 文件编号
    int fd_;

    string stream_info_;
    bool locked_;
    char state_;

    uint16_t record_fragment_count_;

    UTime start_time_;
    UTime end_time_; 
    UTime i_frame_start_time_;
    UTime i_frame_end_time_;

    uint32_t record_offset_;

    RecordFile(string base_name, uint32_t number);

    /* 1. 将该录像文件对应的索引文件中的描述段清零 */
    /* 2. 清零内存中的数据*/
    int32_t ClearIndex();
    int32_t ClearMemInfo();
    int32_t Clear();

    int32_t EncodeRecordFileInfoIndex(char * record_file_buffer, uint32_t record_file_length);
    int32_t EncodeRecordFragInfoIndex(char *record_frag_info_buffer, uint32_t record_frag_info_length);
    int32_t BuildIndex(char *record_file_buffer, uint32_t record_file_length, char *record_frag_info_buffer,
                            uint32_t record_frag_info_length, uint32_t *record_frag_number);

    int32_t Append(string &buffer, uint32_t length, BufferTimes &times);

    int32_t FinishWrite();
};


}
#endif
