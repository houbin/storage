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
#include "../include/storage.h"

using namespace util;

namespace storage
{

RecordFile::RecordFile(Logger *logger, string base_name, uint32_t number)
: logger_(logger), base_name_(base_name), number_(number), rwlock_("RecordFile::rwlocker"), write_fd_(-1), read_fd_(-1), read_count_(0),
locked_(false), had_frame_(false), record_fragment_count_(0), frag_start_offset_(0), record_offset_(0)
{
    ZeroRecordFileTimes();
}

int32_t RecordFile::OpenFd(bool for_write)
{
    char buffer[32] = {0};

    snprintf(buffer, 32, "record_%05d", number_);
    string record_file_path(base_name_);
    record_file_path.append(buffer);

    if (for_write)
    {
        assert(write_fd_ < 0);
        write_fd_ = open(record_file_path.c_str(), O_WRONLY);
        assert(write_fd_ >= 0);
    }
    else
    {
        assert(read_fd_ < 0);
        read_fd_ = open(record_file_path.c_str(), O_RDONLY);
        assert(read_fd_ >= 0);
    }

    return 0;
}

int32_t RecordFile::Clear()
{
    //Log(logger_, "clear record file index memory info");

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
    
    string temp;
    temp.swap(this->stream_info_);

    this->locked_ = false;
    this->had_frame_ = false;
    this->record_fragment_count_ = 0;
    UTime time_zero(0, 0);
    frag_start_time_ = time_zero;
    frag_end_time_ = time_zero;
    ZeroRecordFileTimes();
    this->frag_start_offset_ = 0;
    this->record_offset_ = 0;

    return 0;
}

int32_t RecordFile::UpdateTimes(BufferTimes &update)
{
    // update record file times
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

    // update frag times
    if (frag_start_time_ == 0 && update.start_time != 0)
    {
        frag_start_time_ = update.start_time;
    }

    if (frag_end_time_ < update.end_time)
    {
        frag_end_time_ = update.end_time;
    }
    
    return 0;
}

int32_t RecordFile::ZeroRecordFileTimes()
{
    UTime temp(0, 0);
    this->start_time_ = temp;
    this->end_time_ = temp;
    this->i_frame_start_time_ = temp;
    this->i_frame_end_time_ = temp;

    return 0;
}

int32_t RecordFile::EncodeRecordFileInfoIndex(char *record_file_info_buffer, uint32_t record_file_info_length)
{
//    Log(logger_, "encode record file info index, stream info is %s, record_fragment_count_ is %d, start_time is %d.%d, end_time is %d.%d, \
//record_offset_ is %d", stream_info_.c_str(), record_fragment_count_, start_time_.tv_sec, start_time_.tv_nsec, end_time_.tv_sec,
//end_time_.tv_nsec, record_offset_);

    struct RecordFileInfo record_file_info;
    char *temp = record_file_info_buffer;

    uint32_t length = record_file_info_length - sizeof(record_file_info.length) - sizeof(record_file_info.crc);
    EncodeFixed32(temp, length);
    temp += 4;
    temp += 4;

    char *crc_start = temp;

    memcpy(temp, stream_info_.c_str(), stream_info_.length());
    temp += 64;

    *temp = locked_;
    temp += 1;

    *temp = record_fragment_count_ & 0xff;
    *(temp+1) = record_fragment_count_ >> 8;
    temp += 2;

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

int32_t RecordFile::DecodeRecordFileInfoIndex(char *buffer, uint32_t length)
{
    //Log(logger_, "decode record file info index, length is %d", length);
    assert(buffer != NULL);

    char *temp = NULL;

    temp = buffer;
    char stream_str[64] = {0};
    memcpy(stream_str, buffer, 64);
    stream_info_.assign(stream_str);
    temp += 64;

    locked_ = *temp;
    temp += 1;

    uint32_t a = *temp;
    uint32_t b = *(temp + 1);
    record_fragment_count_ = a | (b << 8);
    temp += 2;

    start_time_.tv_sec = DecodeFixed32(temp);
    temp += 4;
    start_time_.tv_nsec = DecodeFixed32(temp);
    temp += 4;

    end_time_.tv_sec = DecodeFixed32(temp);
    temp += 4;
    end_time_.tv_nsec = DecodeFixed32(temp);
    temp +=4;

    i_frame_start_time_.tv_sec = DecodeFixed32(temp);
    temp += 4;
    i_frame_start_time_.tv_nsec = DecodeFixed32(temp);
    temp += 4;

    i_frame_end_time_.tv_sec = DecodeFixed32(temp);
    temp += 4;
    i_frame_end_time_.tv_nsec = DecodeFixed32(temp);
    temp += 4;

    record_offset_ = DecodeFixed32(temp); 
    temp += 4;

    assert(temp <= buffer + length);

//    Log(logger_, "decode record file ok. stream info: %s, record_fragment_count: %d, start_time: %d.%d, end_time: %d.%d, \
//i_frame_start_time: %d.%d, i_frame_end_time: %d.%d, record_offset_: %d", stream_info_.c_str(), record_fragment_count_, 
//    start_time_.tv_sec, start_time_.tv_nsec, end_time_.tv_sec, end_time_.tv_nsec, i_frame_start_time_.tv_sec, 
//    i_frame_start_time_.tv_nsec, i_frame_end_time_.tv_sec, i_frame_end_time_.tv_nsec, record_offset_);

    return 0;
}

int32_t RecordFile::EncodeRecordFragInfoIndex(char *record_frag_info_buffer, uint32_t record_frag_info_length)
{
//    Log(logger_, "encode record frag info index, start time: %d.%d, end time: %d.%d, frag start offset: %d, frag end offset: %d", 
//        frag_start_time_.tv_sec, frag_start_time_.tv_nsec, frag_end_time_.tv_sec, frag_end_time_.tv_nsec, frag_start_offset_, record_offset_);

    struct RecordFragmentInfo record_frag_info;
    char *temp = record_frag_info_buffer;
    
    uint32_t length = record_frag_info_length - sizeof(record_frag_info.length) - sizeof(record_frag_info.crc);
    EncodeFixed32(temp, length);
    temp += sizeof(record_frag_info.length);

    temp += sizeof(record_frag_info.crc);

    char *crc_start = temp;

    EncodeFixed32(temp, frag_start_time_.tv_sec);
    temp += 4;
    EncodeFixed32(temp, frag_start_time_.tv_nsec);
    temp += 4;

    EncodeFixed32(temp, frag_end_time_.tv_sec);
    temp += 4;
    EncodeFixed32(temp, frag_end_time_.tv_nsec);
    temp += 4;

    EncodeFixed32(temp, frag_start_offset_);
    temp += 4;
    EncodeFixed32(temp, record_offset_);
    temp += 4;

    uint32_t crc = crc32c::Value(crc_start, length);
    EncodeFixed32(crc_start - 4, crc);

    return 0;
}

int32_t RecordFile::DecodeRecordFragInfoIndex(char *buffer, uint32_t length, RecordFragmentInfo &frag_info)
{
    assert(buffer != NULL);
    
    char *temp = buffer;
    uint32_t actual_length = 0;

    actual_length = DecodeFixed32(temp);
    temp += 4;

    uint32_t expected_length = sizeof(RecordFragmentInfo) - sizeof(frag_info.length) - sizeof(frag_info.crc);
    if (expected_length != actual_length)
    {
        return -ERR_CRC_CHECK;
    }

    uint32_t expected_crc = DecodeFixed32(temp);
    temp += 4;

    uint32_t actual_crc = crc32c::Value(temp, actual_length);
    assert(actual_crc == expected_crc);
    
    frag_info.start_time.tv_sec = DecodeFixed32(temp);
    temp += 4;
    frag_info.start_time.tv_nsec = DecodeFixed32(temp);
    temp += 4;

    frag_info.end_time.tv_sec = DecodeFixed32(temp);
    temp += 4;
    frag_info.end_time.tv_nsec = DecodeFixed32(temp);
    temp += 4;

    frag_info.frag_start_offset = DecodeFixed32(temp);
    temp += 4;
    frag_info.frag_end_offset = DecodeFixed32(temp);
    temp += 4;

    //Log(logger_, "decode record frag info index, time start %d.%d, end %d.%d. offset start %d, end %d",
    //    frag_info.start_time.tv_sec, frag_info.start_time.tv_nsec, frag_info.end_time.tv_sec, frag_info.end_time.tv_nsec,
    //    frag_info.frag_start_offset, frag_info.frag_end_offset);
    assert(temp <= buffer + length);
    
    return 0;
}

int32_t RecordFile::GetAllFragInfoEx(deque<RecordFragmentInfo> &frag_info_queue)
{
    int32_t ret;

    RWLock::RDLocker lock(rwlock_);

    /* need to read fragment info in index file */ 
    IndexFile *index_file = NULL; 
    ret = index_file_manager->Find(base_name_, &index_file); 
    assert(ret == 0); 
    
    uint32_t file_counts = index_file->GetFileCounts();
    uint32_t frag_info_offset = file_counts * sizeof(RecordFileInfo) + number_ * kStripeCount * sizeof(RecordFragmentInfo);
    uint32_t frag_info_length = sizeof(RecordFragmentInfo) * record_fragment_count_;
    
    char *record_frag_info_buffer = (char *)malloc(frag_info_length);
    assert(record_frag_info_buffer != NULL); 
    memset(record_frag_info_buffer, 0, frag_info_length); 
    
    ret = index_file->Read(record_frag_info_buffer, frag_info_length, frag_info_offset); 
    if (ret != 0) 
    { 
        Log(logger_, "index file read length is %d, offset is %d error", frag_info_length, frag_info_offset); 
        assert(ret != 0); 
    } 
    
    for(int i = 0; i < record_fragment_count_; i++)
    {
        RecordFragmentInfo temp_frag_info = {0};
        char *temp_buffer = record_frag_info_buffer + i * sizeof(RecordFragmentInfo);

        ret = DecodeRecordFragInfoIndex(temp_buffer, sizeof(RecordFragmentInfo), temp_frag_info);
        if (ret == -ERR_CRC_CHECK)
        {
            Log(logger_, "found crc check error, i is %d", i);
            safe_free(record_frag_info_buffer);
            return 0;
        }

        frag_info_queue.push_back(temp_frag_info);
    }

    safe_free(record_frag_info_buffer);
    return 0;
}

int32_t RecordFile::GetAllFragInfo(deque<FRAGMENT_INFO_T> &frag_info_queue)
{
    //Log(logger_, "get all record frag info index");

    int32_t ret;

    deque<RecordFragmentInfo> temp_queue;
    ret = GetAllFragInfoEx(temp_queue);
    assert(ret == 0);

    //Log(logger_, "size of temp_queue is %d", temp_queue.size());

    while(!temp_queue.empty())
    {
        RecordFragmentInfo temp_frag = temp_queue.front();
        temp_queue.pop_front();

        FRAGMENT_INFO_T frag_info = {0};
        frag_info.start_time.seconds = temp_frag.start_time.tv_sec;
        frag_info.start_time.nseconds = temp_frag.start_time.tv_nsec;
        frag_info.end_time.seconds = temp_frag.end_time.tv_sec;
        frag_info.end_time.nseconds = temp_frag.end_time.tv_nsec;

        frag_info_queue.push_back(frag_info);
    }

    return 0;
}

int32_t RecordFile::DecodeHeader(char *header, FRAME_INFO_T *frame)
{
    assert(header != NULL);
    assert(frame != NULL);
    
    char *temp = header;

    uint32_t magic_code = DecodeFixed32(temp);
    temp += 4;
    if (magic_code != kMagicCode)
    {
        Log(logger_, "decode header, no magic code");
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

    //Log(logger_, "decode header ok, type %d, frame time %d.%d, stamp %d, size %d", frame->type, 
    //frame->frame_time.seconds, frame->frame_time.nseconds, frame->stamp, frame->size);
    return 0;
}

int32_t RecordFile::GetStampStartAndEndOffset(UTime &stamp, uint32_t &frag_start_offset, uint32_t &frag_end_offset)
{
    int32_t ret;
    deque<RecordFragmentInfo> temp_queue;

    /* get all frag info */
    ret = GetAllFragInfoEx(temp_queue);
    assert (ret == 0);

    deque<RecordFragmentInfo>::iterator iter = temp_queue.begin();
    for(; iter != temp_queue.end(); iter++)
    {
        RecordFragmentInfo frag_info = *iter;

        if (stamp >= frag_info.start_time && stamp <= frag_info.end_time)
        {
            frag_start_offset = frag_info.frag_start_offset;
            frag_end_offset = frag_info.frag_end_offset;
            break;
        }
    }

    if (iter == temp_queue.end())
    {
        return -1;
    }

    return 0;
}

int32_t RecordFile::ReadFrame(uint32_t offset, FRAME_INFO_T *frame)
{
    assert(frame != NULL);
    //Log(logger_, "read frame");

    int ret = 0;

    assert(read_fd_ >= 0);
    if (offset >= record_offset_)
    {
        Log(logger_, "offset %d, record offset %d, read reach to end");
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

bool RecordFile::CanRecycle()
{
    //Log(logger_, "check recycle");

    RWLock::RDLocker locker(rwlock_);
    if (read_fd_ < 0 && write_fd_ < 0 && locked_ == false)
    {
        return true;
    }

    return false;
}

int32_t RecordFile::BuildIndex(char *record_file_info_buffer, uint32_t record_file_info_length, char *record_frag_info_buffer,
                                uint32_t record_frag_info_length, uint32_t *record_frag_number)
{
    //Log(logger_, "build index");

    RWLock::RDLocker locker(rwlock_);
    if (had_frame_ == false)
    {
        return -ERR_RECORD_NO_WRITE;
    }

    EncodeRecordFragInfoIndex(record_frag_info_buffer, record_frag_info_length);
    EncodeRecordFileInfoIndex(record_file_info_buffer, record_file_info_length);
    *record_frag_number = record_fragment_count_;

    return 0;
}

int32_t RecordFile::Append(char *write_buffer, uint32_t length, BufferTimes &update)
{
    int ret;
    bool open_fd = false;

    //Log(logger_, "write file %srecord_%05d, offset is %d, length is %d", base_name_.c_str(), number_, record_offset_, length);

    RWLock::WRLocker lock(rwlock_);

    if (write_fd_ < 0)
    {
        int32_t ret = OpenFd(true);
        assert(ret == 0);
        lseek(write_fd_, record_offset_, SEEK_SET);

        record_fragment_count_ += 1;
        assert(record_fragment_count_ <= 256);

        // update start and end offset, zero times
        frag_start_offset_ = record_offset_;

        open_fd = true;
    }

    ret = write(write_fd_, write_buffer, length);
    if (ret != length)
    {
        Log(logger_, "write return %d, errno msg is %s", ret, strerror(errno));
        assert(ret == (int)length);
    }

    fdatasync(write_fd_);
    UpdateTimes(update);

    record_offset_ += length;

    if (open_fd)
    {
        had_frame_ = true;
    }

    //Log(logger_, "write file %srecord_%05d ok, record offset is %d, write length is %d",
    //                    base_name_.c_str(), number_, record_offset_, length);

    return 0;
}

int32_t RecordFile::FinishWrite()
{
    //Log(logger_, "finish write");
    
    RWLock::WRLocker lock(rwlock_);
    if (write_fd_ > 0)
    {
        close(write_fd_);
        write_fd_ = -1;
    }

    had_frame_ = false;

    UTime temp(0, 0);
    frag_start_time_ = temp;
    frag_end_time_ = temp;

    return 0;
}

int32_t RecordFile::SeekStampOffset(UTime &stamp, uint32_t &seek_start_offset, uint32_t &seek_end_offset)
{
    //Log(logger_, "get stamp offset");
    int32_t ret = 0;
    uint32_t frag_start_offset = 0;
    uint32_t frag_end_offset = 0;

    {
        RWLock::WRLocker lock(rwlock_);

        read_count_++;
        if (read_fd_ < 0)
        {
            ret = OpenFd(false);
            assert(ret == 0);
        }
    
    }

    {
        RWLock::RDLocker lock(rwlock_);
        ret = GetStampStartAndEndOffset(stamp, frag_start_offset, frag_end_offset);
        if (ret != 0)
        {
            return -ERR_ITEM_NOT_FOUND;
        }
    }
    
    seek_end_offset = frag_end_offset;

    uint32_t stamp_offset = frag_start_offset;
    while(stamp_offset < frag_end_offset)
    {
        char header[kHeaderSize] = {0};
        FRAME_INFO_T frame = {0};
        
        ret = pread(read_fd_, header, kHeaderSize, stamp_offset);
        assert(ret == kHeaderSize);

        ret = DecodeHeader(header, &frame);
        if (ret == -ERR_NO_MAGIC_CODE)
        {
            return ret;
        }

        UTime frame_time(frame.frame_time.seconds, frame.frame_time.nseconds);
        if (frame_time >= stamp)
        {
            seek_start_offset = stamp_offset;
            return 0;
        }

        stamp_offset += kHeaderSize;
        stamp_offset += frame.size;
    }

    if (stamp_offset >= record_offset_)
    {
        assert("?" == 0);
        return -ERR_READ_REACH_TO_END;
    }

    return 0;
}

int32_t RecordFile::FinishRead()
{
    //Log(logger_, "finish read");

    RWLock::WRLocker lock(rwlock_);
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

