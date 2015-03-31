#include "record_reader.h"
#include "index_file.h"

namespace storage
{

RecordReader::RecordReader(Logger *logger, RecordFileMap *file_map)
: logger_(logger), file_map_(file_map), record_file_(NULL), read_offset_(0), read_end_offset_(0)
{
    memset((void *)&current_o_frame_, 0, sizeof(FRAME_INFO_T));
}

int32_t RecordReader::Seek(UTime &stamp)
{
    int32_t ret;
    uint32_t seek_start_offset = 0;
    uint32_t seek_end_offset = 0;
    RecordFile *record_file = NULL;

    ret = file_map_->SeekStampOffset(stamp, &record_file, seek_start_offset, seek_end_offset);
    if (ret != 0)
    {
        return ret;
    }
    
    record_file_ = record_file;
    read_offset_ = seek_start_offset;
    read_end_offset_ = seek_end_offset;

    LOG_DEBUG(logger_, "seek record file %srecord_%05d, start offset %d, end offset %d", record_file_->base_name_.c_str(), 
                            record_file_->number_, read_offset_, read_end_offset_);

    return 0;
}

int32_t RecordReader::ReadFrame(FRAME_INFO_T *frame)
{
    int32_t ret;

    if (record_file_ == NULL)
    {
        return -ERR_SEEK_FAILED;
    }

    while(read_offset_ < read_end_offset_)
    {
        ret = record_file_->ReadFrame(read_offset_, frame);
        if (ret != 0)
        {
            LOG_WARN(logger_, "read frame error, read_offset %d, ret %d", read_offset_, ret);
            return ret;
        }

        if (frame->type == JVN_DATA_O)
        {
           /* o frame no change */
           if ((current_o_frame_.type == JVN_DATA_O)
               && (current_o_frame_.size == frame->size)
               && (memcmp(current_o_frame_.buffer, frame->buffer, frame->size)) == 0)
           {
               LOG_INFO(logger_, "o frame not changed, so skip this o frame");
               read_offset_ += kHeaderSize;
               read_offset_ += frame->size;
               continue;
           }
            
            /* o frame have change */
            current_o_frame_.type = frame->type;
            current_o_frame_.frame_time = frame->frame_time;
            current_o_frame_.stamp = frame->stamp;
            current_o_frame_.size = frame->size;
            if (current_o_frame_.buffer == NULL)
            {
                current_o_frame_.buffer = (char *)malloc(frame->size);
                assert(current_o_frame_.buffer != NULL);
            }
            memcpy(current_o_frame_.buffer, frame->buffer, frame->size);
            LOG_INFO(logger_, "o frame changed");
        }

        break;
    }

    if (read_offset_ >= read_end_offset_)
    {
        LOG_ERROR(logger_, "read offset %d, read end offset %d", read_offset_, read_end_offset_);
        return -ERR_READ_OVER_THAN_END_OFFSET;
    }

    /* return frame */
    read_offset_ += kHeaderSize;
    read_offset_ += frame->size;

    LOG_DEBUG(logger_, "read frame record file %srecord_%05d, read_offset_ %d, record_offset_ %d", record_file_->base_name_.c_str(),
                    record_file_->number_, read_offset_, record_file_->record_offset_);

    return 0;
}

int32_t RecordReader::Close()
{
    safe_free(current_o_frame_.buffer);

    if (record_file_ != NULL)
    {
        Log(logger_, "finished to read record file, %srecord_%05d, current read offset %d, read end offset %d", 
        record_file_->base_name_.c_str(), record_file_->number_, read_offset_, read_end_offset_);

        int32_t ret;
        ret = file_map_->FinishReadRecordFile(record_file_);
        assert(ret == 0);
    }

    return 0;
}

void RecordReader::Shutdown()
{
    return;
}

}
