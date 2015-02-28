#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "../util/clock.h"
#include "../util/coding.h"
#include "../util/crc32c.h"
#include "store_types.h"
#include "record_file.h"
#include "index_file.h"

using namespace util;

namespace storage
{

RecordFile::RecordFile(Logger *logger, string base_name, uint32_t number)
: logger_(logger), base_name_(base_name), number_(number), write_fd_(-1), read_fd_(-1), read_count_(0),
locked_(false), record_fragment_count_(0), start_time_(0, 0), end_time_(0, 0),
i_frame_start_time_(0, 0), i_frame_end_time_(0, 0), record_offset_(0)
{

}

int32_t RecordFile::Clear()
{
    Log(logger_, "clear record file index memory info");

    if (this->write_fd_ >= 0)
    {
        close(this->write_fd_);
    }

    if (this->read_fd_ >= 0)
    {
        close(this->read_fd_);
    }
    
    this->write_fd_ = -1;
    this->read_fd_ = -1;
    this->read_count_= 0;
    
    this->stream_info_.clear();
    this->locked_ = false;
    this->record_fragment_count_ = 0;
    UTime temp(0, 0);
    this->start_time_ = temp;
    this->end_time_ = temp;
    this->i_frame_start_time_ = temp;
    this->i_frame_end_time_ = temp;
    this->record_offset_ = 0;

    return 0;
}

bool RecordFile::CheckRecycle()
{
    Log(logger_, "check recycle");

    if (read_fd_ < 0 && write_fd_ < 0 && locked_ == true)
    {
        return false;
    }

    return true;
}

int32_t RecordFile::EncodeRecordFileInfoIndex(char *record_file_info_buffer, uint32_t record_file_info_length)
{
    Log(logger_, "encode record file info index");

    struct RecordFileInfo record_file_info;
    char *temp = record_file_info_buffer;

    uint32_t length = record_file_info_length - sizeof(record_file_info.length) - sizeof(record_file_info.crc);
    EncodeFixed32(temp, length);
    temp += sizeof(record_file_info.length);
    temp += sizeof(record_file_info.crc);

    char *crc_start = temp;

    memcpy(temp, stream_info_.c_str(), stream_info_.length());
    temp += sizeof(record_file_info.stream_info);

    *temp = locked_;
    temp += sizeof(record_file_info.locked);

    *temp = record_fragment_count_ & 0xff;
    *(temp+1) = record_fragment_count_ >> 8;
    temp += sizeof(record_file_info.record_fragment_counts);

    EncodeFixed32(temp, start_time_.tv_sec);
    temp += 4;
    EncodeFixed32(temp, start_time_.tv_nsec);
    temp += 4;

    EncodeFixed32(temp, end_time_.tv_sec);
    temp += 4;
    EncodeFixed32(temp, end_time_.tv_nsec);
    temp += 4;

    EncodeFixed32(temp, i_frame_start_time_.tv_sec);
    temp += 4;
    EncodeFixed32(temp, i_frame_start_time_.tv_nsec);
    temp += 4;

    EncodeFixed32(temp, i_frame_end_time_.tv_sec);
    temp += 4;
    EncodeFixed32(temp, i_frame_end_time_.tv_nsec);
    temp += 4;

    EncodeFixed32(temp, record_offset_);
    temp += 4;

    uint32_t crc = crc32c::Value(crc_start, length);
    EncodeFixed32(crc_start - 4, crc);

    return 0;
}

int32_t RecordFile::EncodeRecordFragInfoIndex(char *record_frag_info_buffer, uint32_t record_frag_info_length)
{
    Log(logger_, "encode record frag info index");

    struct RecordFragmentInfo record_frag_info;
    char *temp = record_frag_info_buffer;
    
    uint32_t length = record_frag_info_length - sizeof(record_frag_info.length) - sizeof(record_frag_info.crc);
    temp += sizeof(record_frag_info.length);
    temp += sizeof(record_frag_info.crc);

    char *crc_start = temp;


    EncodeFixed32(temp, start_time_.tv_sec);
    temp += 4;
    EncodeFixed32(temp, start_time_.tv_nsec);
    temp += 4;

    EncodeFixed32(temp, end_time_.tv_sec);
    temp += 4;
    EncodeFixed32(temp, end_time_.tv_nsec);
    temp += 4;

    EncodeFixed32(temp, i_frame_start_time_.tv_sec);
    temp += 4;
    EncodeFixed32(temp, i_frame_start_time_.tv_nsec);
    temp += 4;

    EncodeFixed32(temp, i_frame_end_time_.tv_sec);
    temp += 4;
    EncodeFixed32(temp, i_frame_end_time_.tv_nsec);
    temp += 4;

    uint32_t crc = crc32c::Value(crc_start, length);
    EncodeFixed32(crc_start - 4, crc);

    return 0;
}

int32_t RecordFile::BuildIndex(char *record_file_info_buffer, uint32_t record_file_info_length, char *record_frag_info_buffer,
                                uint32_t record_frag_info_length, uint32_t *record_frag_number)
{
    Log(logger_, "build index");

    EncodeRecordFileInfoIndex(record_file_info_buffer, record_file_info_length);
    EncodeRecordFragInfoIndex(record_frag_info_buffer, record_frag_info_length);
    *record_frag_number = record_fragment_count_;

    return 0;
}

int32_t RecordFile::Append(string &buffer, uint32_t length, BufferTimes &update)
{
    int ret;
    Log(logger_, "append length is %d", length);

    if (write_fd_ < 0)
    {
        char buffer[32] = {0};
        snprintf(buffer, 32, "record_%05d", number_);

        string record_file_path(base_name_);
        record_file_path.append(buffer);

        write_fd_ = open(record_file_path.c_str(), O_WRONLY);
        assert(write_fd_ > 0);
        lseek(write_fd_, record_offset_, SEEK_SET);

        record_fragment_count_ += 1;
        assert(record_fragment_count_ <= 256);
    }

    assert(write_fd_ >= 0);
    
    ret = write(write_fd_, buffer.c_str(), length);
    if (ret != length)
    {
        Log(logger_, "write return %d, errno msg is %s", ret, strerror(errno));
        assert(ret == (int)length);
    }

    fdatasync(write_fd_);

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

int32_t RecordFile::FinishWrite()
{
    Log(logger_, "finish write");
    
    if (write_fd_ > 0)
    {
        close(write_fd_);
        write_fd_ = -1;
    }

    return 0;
}

int32_t RecordFile::DecodeHeader(char *header, FRAME_INFO_T *frame)
{
    assert(header != NULL);
    assert(frame != NULL);
    Log(logger_, "decode header");
    
    char *temp = header;

    uint32_t magic_code = DecodeFixed32(temp);
    temp += 4;
    if (magic_code != kMagicCode)
    {
        return -ERR_NO_MAGIC_CODE;
    }

    frame->type = DecodeFixed32(temp);
    temp += 4;

    frame->frame_time.seconds = DecodeFixed32(temp);
    temp += 4;

    frame->frame_time.nseconds = DecodeFixed32(temp);
    temp += 4;

    frame->stamp = DecodeFixed64(temp);
    temp += 8;

    frame->size = DecodeFixed32(temp);
    temp += 4;

    return 0;
}

int32_t RecordFile::SeekStampOffset(UTime &stamp, uint32_t *offset)
{
    Log(logger_, "get stamp offset");
    uint32_t ret = 0;
    uint32_t stamp_offset = 0;

    read_count_++;
    if (read_fd_ < 0)
    {
        char buffer[32] = {0};
        snprintf(buffer, 32, "record_%05d", number_);

        string record_file_path(base_name_);
        record_file_path.append(buffer);
        read_fd_ = open(record_file_path.c_str(), O_RDONLY);
        assert(read_fd_ > 0);
    }

    while(stamp_offset < record_offset_)
    {
        char header[kHeaderSize] = {0};
        FRAME_INFO_T frame = {0};
        
        ret = pread(read_fd_, header, kHeaderSize, stamp_offset);
        assert(ret == kHeaderSize);

        ret = DecodeHeader(header, &frame);
        if (ret != 0)
        {
            return ret;
        }

        if (frame.type == JVN_DATA_O)
        {
            UTime frame_time(frame.frame_time.seconds, frame.frame_time.nseconds);
            if (frame_time >= stamp)
            {
                *offset = stamp_offset;
                return 0;
            }
        }

        stamp_offset += kHeaderSize;
        stamp_offset += frame.size;
    }

    if (stamp_offset >= record_offset_)
    {
        return -ERR_READ_REACH_TO_END;
    }

    return 0;
}

int32_t RecordFile::ReadFrame(uint32_t read_offset, FRAME_INFO_T *frame)
{
    assert(frame != NULL);
    Log(logger_, "read frame");

    int ret = 0;

    assert(read_fd_ >= 0);
    if (read_offset >= record_offset_)
    {
        Log(logger_, "read reach to end");
        return -ERR_READ_REACH_TO_END;
    }

    char header[kHeaderSize] = {0};
    ret = pread(read_fd_, header, kHeaderSize, offset);
    assert(ret == (int)kHeaderSize);

    {
        uint32_t ret = DecodeHeader(header, frame);
        if (ret != 0)
        {
            return ret;
        }
    }

    ret = pread(read_fd_, frame->buffer, frame->size, offset + kHeaderSize);
    assert(ret == (int)frame->size);

    return 0;
}

int32_t RecordFile::FinishRead()
{
    Log(logger_, "finish read");
    read_count_--;
    assert(read_count_ >= 0);
    if (read_count_ == 0)
    {
        close(read_fd_);
        read_fd_ = -1;
    }

    return 0;
}

}

