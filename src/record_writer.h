#ifndef STORAGE_RECORD_WRITER_H_
#define STORAGE_RECORD_WRITER_H_

#include <vector>
#include <deque>
#include "../include/errcode.h"
#include "../include/storage_api.h"
#include "../util/utime.h"
#include "../util/logger.h"
#include "../util/thread.h"
#include "../util/rwlock.h"
#include "../util/coding.h"
#include "store_types.h"
#include "config_opts.h"
#include "id_center.h"
#include "free_file_table.h"

using namespace std;
using namespace util;

namespace storage
{

class RecordFile;
class RecordFileMap;

class C_WriteIndexTick; 

class RecordWriter : public Thread
{
private:
    Logger *logger_;
    string stream_info_;
    RecordFileMap *file_map_;

    Mutex queue_mutex_;
    Cond queue_cond_;
    deque<WriteOp*> write_op_queue_;

    // protect blow variable
    Mutex writer_mutex_;
    RecordFile *record_file_;

    char *buffer_;
    uint32_t buffer_write_offset_;
    BufferTimes buffer_times_;

    FRAME_INFO_T *current_o_frame_;

    C_WriteIndexTick *write_index_event_;

    bool stop_;

public:
    RecordWriter(Logger *logger, string stream_info, RecordFileMap *file_map_);

    bool IsStopped();

    int32_t Enqueue(WriteOp *write_op);
    int32_t Dequeue(WriteOp **write_op);

    int32_t EncodeFrameHeader(char *buffer, FRAME_INFO_T *frame);
    int32_t EncodeFrame(bool add_o_frame, FRAME_INFO_T *frame, char *temp_buffer);
    int32_t UpdateBufferTimes(uint32_t type, UTime &time);
    int32_t WriteBuffer(uint32_t write_length);
    int32_t BuildRecordFileIndex(RecordFile *record_file, char *record_file_info_buffer, uint32_t record_file_info_length,
                        char *record_frag_info_buffer, uint32_t record_frag_info_length, uint32_t *record_frag_info_number);

    int32_t WriteRecordFileIndex(int r);
    int32_t ResetWriteIndexEvent(uint32_t after_seconds);
    int32_t DoWriteIndexEvent(bool again);

    int32_t PrepareRecordFile(bool need_new_file, UTime &stamp);

    void Start();
    void *Entry();
    void Stop();

    void Shutdown();
};

class C_WriteIndexTick: public Context
{
    RecordWriter *writer_;

public:

    C_WriteIndexTick(RecordWriter *writer)
    : writer_(writer)
    {}

    void Finish(int r)
    {
        writer_->WriteRecordFileIndex(r);
        return;
    }
};

}

#endif

