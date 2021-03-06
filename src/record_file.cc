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
#include "storage.h"
#include "libaio_wrap.h"

using namespace util;

namespace storage
{

RecordFile::RecordFile(Logger *logger, string base_name, uint32_t number)
: logger_(logger), base_name_(base_name), number_(number), rwlock_("RecordFile::rwlocker"),
write_fd_(-1), write_aio_ctx_(0), read_aio_ctx_mutex_("RecordFile::read_aio_ctx_mutex"), read_fd_(-1), read_count_(0), read_aio_ctx_(0),
locked_(false), have_write_frame_(false), record_fragment_count_(0), frag_start_offset_(0), record_offset_(0)
{
    ZeroRecordFileTimes();
}

int32_t RecordFile::OpenFd(bool for_write)
{
    int ret = 0;
    char buffer[32] = {0};

    snprintf(buffer, 32, "record_%05d", number_);
    string record_file_path(base_name_);
    record_file_path.append(buffer);

    if (for_write)
    {
        assert(write_fd_ < 0);
        write_fd_ = open(record_file_path.c_str(), O_WRONLY | O_DIRECT | O_DSYNC, 0644);
        assert(write_fd_ >= 0);

        record_fragment_count_ += 1;
        assert(record_fragment_count_ <= 256);
        frag_start_offset_ = record_offset_;

        // setup write_aio_ctx_
        ret = io_setup(8, &write_aio_ctx_);
        if (ret != 0)
        {
            LOG_FATAL(logger_, "io_setup write_aio_ctx error");
            assert(ret == 0);
        }
    }
    else
    {
        assert(read_fd_ < 0);
        read_fd_ = open(record_file_path.c_str(), O_RDONLY);
        assert(read_fd_ >= 0);

        // setup read_aio_ctx_
        ret = io_setup(8, &read_aio_ctx_);
        if (ret != 0)
        {
            LOG_FATAL(logger_, "io_setup read_aio_ctx error");
            assert(ret == 0);
        }
    }

    return 0;
}

int32_t RecordFile::Clear()
{
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
    this->have_write_frame_ = false;
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

//    Log(logger_, "decode record file ok. stream info: %s, record_fragment_count: %d, start_time: %d.%d, end_time: %d.%d, \ ///
//i_frame_start_time: %d.%d, i_frame_end_time: %d.%d, record_offset_: %d", stream_info_.c_str(), record_fragment_count_, ///
//    start_time_.tv_sec, start_time_.tv_nsec, end_time_.tv_sec, end_time_.tv_nsec, i_frame_start_time_.tv_sec, ///
//    i_frame_start_time_.tv_nsec, i_frame_end_time_.tv_sec, i_frame_end_time_.tv_nsec, record_offset_);///

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

    uint16_t read_index_file_frag_count = record_fragment_count_;
    if (have_write_frame_)
    {
        read_index_file_frag_count -= 1;
    }
    
    uint32_t file_counts = index_file->GetFileCounts();
    uint32_t frag_info_offset = file_counts * sizeof(RecordFileInfo) + number_ * kStripeCount * sizeof(RecordFragmentInfo);
    uint32_t frag_info_length = sizeof(RecordFragmentInfo) * read_index_file_frag_count;
    
    char *record_frag_info_buffer = (char *)malloc(frag_info_length);
    assert(record_frag_info_buffer != NULL); 
    memset(record_frag_info_buffer, 0, frag_info_length); 
    
    ret = index_file->Read(record_frag_info_buffer, frag_info_length, frag_info_offset); 
    if (ret != 0) 
    { 
        LOG_WARN(logger_, "maybe bad block of disk, ret %d, read length %d, offset %d", ret, frag_info_length, frag_info_offset); 
        return ret;
    } 
    
    for(int i = 0; i < read_index_file_frag_count; i++)
    {
        RecordFragmentInfo temp_frag_info = {0};
        char *temp_buffer = record_frag_info_buffer + i * sizeof(RecordFragmentInfo);

        ret = DecodeRecordFragInfoIndex(temp_buffer, sizeof(RecordFragmentInfo), temp_frag_info);
        if (ret == -ERR_CRC_CHECK)
        {
            Log(logger_, "found crc check error, i %d, read_index_file_frag_count %d", i, read_index_file_frag_count);
            safe_free(record_frag_info_buffer);
            return 0;
        }

        frag_info_queue.push_back(temp_frag_info);
    }

    if (have_write_frame_)
    {
        RecordFragmentInfo temp_frag_info = {0};
        temp_frag_info.start_time = frag_start_time_;
        temp_frag_info.end_time = frag_end_time_;
        temp_frag_info.frag_start_offset = frag_start_offset_;
        temp_frag_info.frag_end_offset = record_offset_;

        frag_info_queue.push_back(temp_frag_info);
        Log(logger_, "have write frame, push (%d.%d, %d.%d) to frag info queue", frag_start_time_.tv_sec, frag_start_time_.tv_nsec, 
                    frag_end_time_.tv_sec, frag_end_time_.tv_nsec);
    }

    DumpQueue(frag_info_queue);

    LOG_INFO(logger_, "############## record file info ###############");
    LOG_INFO(logger_, "start time %d.%d, end time %d.%d, start offset %d, end offset %d", start_time_.tv_sec, start_time_.tv_nsec,
                end_time_.tv_sec, end_time_.tv_nsec, 0, record_offset_);
    LOG_INFO(logger, "############## record file info end ############");

    safe_free(record_frag_info_buffer);
    return 0;
}

int32_t RecordFile::GetAllFragInfo(deque<FRAGMENT_INFO_T> &frag_info_queue)
{
    int32_t ret;

    deque<RecordFragmentInfo> temp_queue;
    ret = GetAllFragInfoEx(temp_queue);
    if (ret != 0)
    {
        return ret;
    }

    while(!temp_queue.empty())
    {
        RecordFragmentInfo temp_frag = temp_queue.front();
        temp_queue.pop_front();

        FRAGMENT_INFO_T frag_info;
        memset(&frag_info, 0, sizeof(FRAGMENT_INFO_T));
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
    if (ret != 0)
    {
        // maybe bad block of disk
        return ret;
    }
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

    int32_t ret = 0;

    Mutex::Locker lock(read_aio_ctx_mutex_);
    assert(read_fd_ >= 0);
    if (offset >= record_offset_)
    {
        LOG_ERROR(logger_, "offset %d, record offset %d, read reach to end");
        return -ERR_READ_REACH_TO_END;
    }

    char header[kHeaderSize] = {0};
    ret = libaio_single_read(read_aio_ctx_, read_fd_, header, kHeaderSize, offset);
    if (ret < 0)
    {
        LOG_ERROR(logger_, "libaio read error, ret %d, offset %d, size %d", ret, offset, kHeaderSize);
        bad_disk_map->AddBadDisk(base_name_);
        return ret;
    }
    else if (ret != (int)kHeaderSize)
    {
        LOG_ERROR(logger_, "reached to end of file, ret %d, offset %d, size %d", ret, offset, kHeaderSize);
        return -ERR_READ_REACH_TO_END;
    }
    assert(ret == (int)kHeaderSize);

    ret = DecodeHeader(header, frame);
    if (ret != 0)
    {
        LOG_ERROR(logger_, "decode header error, ret is %d", ret);
        return ret;
    }

    ret = libaio_single_read(read_aio_ctx_, read_fd_, frame->buffer, frame->size, offset + kHeaderSize);
    if (ret < 0)
    {
        LOG_ERROR(logger_, "libaio read error, ret %d, offset %d, size %d", ret, offset, frame->size);
        bad_disk_map->AddBadDisk(base_name_);
        return ret;
    }
    else if(ret != (int)frame->size)
    {
        LOG_ERROR(logger_, "reached to end of file, ret %d, offset %d, size %d", ret, offset, frame->size);
        return ret;
    }
    assert(ret == (int)frame->size);

    return 0;
}

bool RecordFile::CanRecycle()
{
    RWLock::RDLocker locker(rwlock_);
    if (read_fd_ < 0 && write_fd_ < 0 && locked_ == false)
    {
        return true;
    }

    LOG_DEBUG(logger_, "can't recycle record file, belong [%s], read_fd %d, write_fd %d", stream_info_.c_str(), read_fd_, write_fd_);
    return false;
}

int32_t RecordFile::BuildIndex(char *record_file_info_buffer, uint32_t record_file_info_length, char *record_frag_info_buffer,
                                uint32_t record_frag_info_length, uint32_t *record_frag_number)
{
    //Log(logger_, "build index");

    RWLock::RDLocker locker(rwlock_);
    if (have_write_frame_ == false)
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

    rwlock_.GetWriteLock();

    ret = libaio_single_write(write_aio_ctx_, write_fd_, write_buffer, length, record_offset_);
    if (ret <= 0 || ret != (int32_t)length)
    {
        rwlock_.PutWriteLock();
        LOG_FATAL(logger_, "libaio write error, ret %d, write_fd_ %d, length %u, record_offset_ %u", 
                                ret, write_fd_, length, record_offset_);
        bad_disk_map->AddBadDisk(base_name_);
        return -ERR_BAD_DISK;
    }

    UpdateTimes(update);
    record_offset_ += length;
    have_write_frame_ = true;

    rwlock_.PutWriteLock();

    return 0;
}

int32_t RecordFile::FinishWrite()
{
    RWLock::WRLocker lock(rwlock_);
    if (write_fd_ > 0)
    {
        close(write_fd_);
        write_fd_ = -1;

        io_destroy(write_aio_ctx_);
        write_aio_ctx_ = 0;
    }

    have_write_frame_ = false;
    frag_start_time_ = 0;
    frag_end_time_ = 0;

    return 0;
}

int32_t RecordFile::SeekStampOffset(UTime &stamp, uint32_t &seek_start_offset, uint32_t &seek_end_offset)
{
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

    ret = GetStampStartAndEndOffset(stamp, frag_start_offset, frag_end_offset);
    if (ret != 0)
    {
        return -ERR_ITEM_NOT_FOUND;
    }
    
    seek_end_offset = frag_end_offset;

    Mutex::Locker lock(read_aio_ctx_mutex_);
    uint32_t stamp_offset = frag_start_offset;
    while(stamp_offset < frag_end_offset)
    {
        char header[kHeaderSize] = {0};
        FRAME_INFO_T frame = {0};
        
        ret = libaio_single_read(read_aio_ctx_, read_fd_, header, kHeaderSize, stamp_offset);
        if (ret < 0)
        {
            // maybe bad block of disk
            LOG_WARN(logger_, "libaio read header error, ret %d, dir %s, ", ret, base_name_.c_str());
            bad_disk_map->AddBadDisk(base_name_);
            return ret;
        }
        else if (ret != (int)kHeaderSize)
        {
            LOG_WARN(logger_, "reached to end of file, ret %d, dir %s", ret, base_name_.c_str());
            return -ERR_READ_REACH_TO_END;
        }
        assert(ret == (int)kHeaderSize);

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
    RWLock::WRLocker lock(rwlock_);
    read_count_--;
    assert(read_count_ >= 0);
    if (read_count_ == 0)
    {
        close(read_fd_);
        read_fd_ = -1;

        io_destroy(read_aio_ctx_);
        read_aio_ctx_ = 0;
    }

    return 0;
}

void RecordFile::DumpQueue(deque<RecordFragmentInfo> &temp_queue)
{
    int32_t count = 0;
    LOG_INFO(logger_, "############# dump queue size %d #############", temp_queue.size());

    deque<RecordFragmentInfo>::iterator iter = temp_queue.begin();
    for (; iter != temp_queue.end(); iter++)
    {
        RecordFragmentInfo temp = *iter;
        LOG_INFO(logger_, "fragment %d: start time %d.%d, end time %d.%d, start offset %d, end offset %d", count, temp.start_time.tv_sec,
            temp.start_time.tv_nsec, temp.end_time.tv_sec, temp.end_time.tv_nsec, temp.frag_start_offset, temp.frag_end_offset);

        count ++;
    }

    LOG_INFO(logger_, "############# dump queue end #############");
    return ;
}

}

