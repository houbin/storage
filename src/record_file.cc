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
    this->stream_info_.clear();
    this->locked_ = false;
    this->used_ = false;
    this->record_fragment_count_ = 0;
    UTime temp(0, 0);
    this->start_time_ = temp;
    this->end_time_ = temp;
    this->i_frame_start_time_ = temp;
    this->i_frame_end_time_ = temp;
    this->record_offset_ = 0;

    return 0;
}

int32_t RecordFile::Append(String &buffer, uint32_t length, BufferTimes &times)
{
    int ret;
    Log(logger_, "write stream %s , length %d", buffer.c_str(), length);

    assert(fd_ < 0);
    if (fd_ == 0)
    {
        char buffer[32] = {0};
        snprintf(buffer, 32, "record_%05d", number_);

        string record_file_path(base_name_);
        record_file_path.append(buffer);

        int fd = open(record_file_path.c_str(), O_WRONLY | O_SYNC);
        assert(fd < 0);
    }
    
    ret = write(fd, buffer.c_str(), length);
    assert(ret == length);

    if (!used_)
    {
        used_ = true;
        record_fragment_count_ = 1;
    }

    /* update times */
    if((start_time_ == 0) && (update.start_time != 0))
    {
        start_time_ = update.start_time;
    }

    if ((i_frame_start_time_ == 0) && (update.i_frame_start_time != 0))
    {
        i_frame_start_time_ = update.i_frame_start_time;
    }

    if (end_time_ < update.end_time)
    {
        end_time_ = update.end_time;
    }

    if (i_frame_end_time_ < update.i_frame_end_time)
    {
        i_frame_end_time_ = update.i_frame_end_time;
    }

    record_offset_ += length;

    return 0;
}

}

