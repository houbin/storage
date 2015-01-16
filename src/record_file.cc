#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "record_file.h"
#include "index_file.h"
#include "../util/clock.h"

namespace storage
{

RecordFile::RecordFile(string base_name, uint32_t number)
: base_name_(base_name), number_(number), locked_(false),
used_(false), record_fragment_count_(0), start_time_(0), end_time_(0), record_offset_(0)
{

}

int32_t RecordFile::Clear()
{
    int ret;
    uint32_t offset;
    uint32_t length;
    string index_file;

    Log(logger_, "clear index");

    offset = number_ * sizeof(struct RecordFileInfo);
    length = sizeof(struct RecordFileInfo);
    struct RecordFileInfo *record_file_info = new struct RecordFileInfo();
    memset(record_file_info, 0, sizeof(struct RecordFileInfo));

    Struct IndexFileOp *op = new struct IndexFileOp(base_name_, offset, record_file_info);

    IndexFile *index_file;
    ret = index_file_manager->Find(base_name_, &index_file);
    assert(ret == 0);
    assert(index_file != NULL);
    ret = index_file->EnqueueOp(op);
    assert(ret == 0);

    /* 清零内存中的数据 */
    this->stream_id_ = 0;
    this->locked_ = false;
    this->used_ = false;
    this->record_fragment_count_ = 0;
    UTime temp(0, 0);
    this->start_time_ = temp;
    this->start_frame_offset_ = 0;
    this->end_time_ = temp;
    this->i_frame_start_time_ = temp;
    this->i_frame_end_time_ = temp;
    this->record_offset_ = 0;

    return 0;
}

}

