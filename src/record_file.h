#ifndef STORAGE_RECORD_FILE_
#define STORAGE_RECORD_FILE_

#include <string>
#include <deque>
#include "../util/logger.h"
#include "../util/mutex.h"
#include "store_types.h"

using namespace std;
using namespace util;

namespace storage
{

class RecordFile
{
public: 
    Logger *logger_;

    string base_name_;
    uint32_t number_;
    int write_fd_;
    
    int read_fd_;
    int read_count_;

    string stream_info_;
    bool locked_;

    uint16_t record_fragment_count_;

    UTime start_time_;
    UTime end_time_; 
    UTime i_frame_start_time_;
    UTime i_frame_end_time_;

    uint32_t record_offset_;

    RecordFile(Logger *logger, string base_name, uint32_t number);

    int32_t Clear();

    bool CheckRecycle();

    int32_t EncodeRecordFileInfoIndex(char *buffer, uint32_t length);
    int32_t DecodeRecordFileInfoIndex(char *buffer, uint32_t length);

    int32_t EncodeRecordFragInfoIndex(char *buffer, uint32_t length);
    int32_t DecodeRecordFragInfoIndex(char *buffer, uint32_t length, RecordFragmentInfo &frag_info);
    int32_t GetAllFragInfo(deque<FRAGMENT_INFO_T*> &frag_info_queue);

    int32_t BuildIndex(char *record_file_buffer, uint32_t record_file_length, char *record_frag_info_buffer,
                            uint32_t record_frag_info_length, uint32_t *record_frag_number);

    int32_t Append(string &buffer, uint32_t length, BufferTimes &times);
    int32_t FinishWrite();

    int32_t DecodeHeader(char *header, FRAME_INFO_T *frame);
    int32_t SeekStampOffset(UTime &stamp, uint32_t *offset);
    int32_t ReadFrame(uint32_t id, FRAME_INFO_T *frame);
    int32_t FinishRead();
};

}
#endif

