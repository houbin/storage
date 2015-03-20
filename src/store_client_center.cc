#include "../include/storage.h"
#include "../util/coding.h"
#include "index_file.h"
#include "store_client_center.h"
#include "store_types.h"

namespace storage
{

uint64_t kOpSeq = 0;

// ======================================================= //
//                  record file map
// ======================================================= //
RecordFileMap::RecordFileMap(Logger *logger)
: logger_(logger), rwlock_("RecordFileMap::rwlock")
{

}

bool RecordFileMap::Empty()
{
    RWLock::RDLocker lock(rwlock_);
    return record_file_map_.empty();
}

int32_t RecordFileMap::FindStampRecordFile(UTime &time, RecordFile **record_file)
{
    assert(record_file != NULL);
    //Log(logger_, "find stamp %d.%d", time.tv_sec, time.tv_nsec);

    map<UTime, RecordFile*>::iterator iter_up;

    if (Empty())
    {
        return -ERR_TABLE_IS_EMPTY;
    }

    // judge if stamp < all file stamp
    {
        map<UTime, RecordFile*>::iterator iter = record_file_map_.begin();
        assert(iter != record_file_map_.end());
        if (time < iter->first)
        {
            Log(logger_, "time < all file stamp, find time is %d.%d, but first file start time is %d.%d", 
            time.tv_sec, time.tv_nsec, iter->first.tv_sec, iter->first.tv_nsec);
            return -ERR_STAMP_TOO_SMALL;
        }
    }

    // judge if stamp > all file stamp
    {
        map<UTime, RecordFile*>::reverse_iterator riter = record_file_map_.rbegin();
        RecordFile *record_file = riter->second;
        if (record_file->end_time_ < time)
        {
            Log(logger_, "time > all file stamp, find time is %d.%d, but last file end time is %d.%d", 
            time.tv_sec, time.tv_nsec, record_file->end_time_.tv_sec, record_file->end_time_.tv_nsec);
            return -ERR_STAMP_TOO_BIG;
        }
    }

    iter_up = record_file_map_.upper_bound(time);
    if (iter_up == record_file_map_.end())
    {
        map<UTime, RecordFile*>::reverse_iterator riter = record_file_map_.rbegin();
        assert(riter != record_file_map_.rend());
        *record_file = riter->second;
    }
    else
    {
        iter_up--;
        *record_file = iter_up->second;
    }

    return 0;
}

int32_t RecordFileMap::SelectFragInfoWithStartTime(deque<FRAGMENT_INFO_T> &all_frag_info, UTime &start, deque<FRAGMENT_INFO_T> &select_frag_info)
{
    deque<FRAGMENT_INFO_T>::iterator iter = all_frag_info.begin();
    for(; iter != all_frag_info.end(); iter++)
    {
        FRAGMENT_INFO_T temp = *iter;

        if (temp.end_time < start)
        {
            continue;
        }

        if (temp.start_time <= start && temp.end_time >= start)
        {
            temp.start_time.seconds = start.tv_sec;
            temp.start_time.nseconds = start.tv_nsec;

            select_frag_info.push_back(temp);
            continue;
        }

        select_frag_info.push_back(temp);
    }

    return 0;
}

int32_t RecordFileMap::SelectFragInfoWithStartAndEndTime(deque<FRAGMENT_INFO_T> &all_frag_info, UTime &start, UTime &end, 
                                                            deque<FRAGMENT_INFO_T> &select_frag_info)
{
    deque<FRAGMENT_INFO_T>::iterator iter = all_frag_info.begin();
    for (; iter != all_frag_info.end(); iter++)
    {
        FRAGMENT_INFO_T temp = *iter;

        if (temp.end_time < start)
        {
            continue;
        }

        if ((temp.start_time <= start && temp.end_time >= start)
            && (temp.start_time <= end && temp.end_time >= end))
        {
            temp.start_time.seconds = start.tv_sec;
            temp.start_time.nseconds = start.tv_nsec;
            temp.end_time.seconds = end.tv_sec;
            temp.end_time.nseconds =  end.tv_nsec;

            select_frag_info.push_back(temp);
            break;
        }

        if (temp.start_time <= start && temp.end_time >= start)
        {
            temp.start_time.seconds = start.tv_sec;
            temp.start_time.nseconds = start.tv_nsec;

            select_frag_info.push_back(temp);
            continue;
        }

        if (temp.start_time <= end && temp.end_time >= end)
        {
            temp.end_time.seconds = end.tv_sec;
            temp.end_time.nseconds = end.tv_nsec;

            select_frag_info.push_back(temp);
            break;
        }

        select_frag_info.push_back(temp);
    }

    return 0;
}

int32_t RecordFileMap::SelectFragInfoWithEndTime(deque<FRAGMENT_INFO_T> &all_frag_info, UTime &end, deque<FRAGMENT_INFO_T> &select_frag_info)
{
    deque<FRAGMENT_INFO_T>::iterator iter = all_frag_info.begin();
    for (; iter != all_frag_info.end(); iter++)
    {
        FRAGMENT_INFO_T temp = *iter;

        if (temp.start_time <= end && temp.end_time >= end)
        {
            temp.end_time.seconds = end.tv_sec;
            temp.end_time.nseconds = end.tv_nsec;

            select_frag_info.push_back(temp);
            break;
        }

        select_frag_info.push_back(temp);
    }

    return 0;
}

int32_t RecordFileMap::SelectAllFragInfo(deque<FRAGMENT_INFO_T> &all_frag_info, deque<FRAGMENT_INFO_T> &select_frag_info)
{
    deque<FRAGMENT_INFO_T>::iterator iter = all_frag_info.begin();
    for (; iter != all_frag_info.end(); iter++)
    {
        select_frag_info.push_back(*iter);
    }

    return 0;
}

// need read lock before call this function
void RecordFileMap::Dump()
{
    Log(logger_, "dump all record file in map");
    map<UTime, RecordFile*>::iterator iter = record_file_map_.begin();
    for (; iter != record_file_map_.end(); iter++)
    {
        RecordFile *record_file = iter->second;

        Log(logger_, " record file info: stream info %s, record_fragment_count %d, start_time %d.%d, end_time %d.%d, record_offset %d", 
                    record_file->stream_info_.c_str(), record_file->record_fragment_count_,
                    record_file->start_time_.tv_sec, record_file->start_time_.tv_nsec,
                    record_file->end_time_.tv_sec, record_file->end_time_.tv_nsec,
                    record_file->record_offset_);
    }

    return;
}

int32_t RecordFileMap::ListRecordFragments(UTime &start, UTime &end, deque<FRAGMENT_INFO_T> &frag_info_queue)
{
    Log(logger_, "list record fragments from time %d.%d to %d.%d", start.tv_sec, start.tv_nsec, end.tv_sec, end.tv_nsec);

    int32_t ret;
    RecordFile *start_rf = NULL;

    RWLock::RDLocker lock(rwlock_);

    if (Empty())
    {
        return -ERR_TABLE_IS_EMPTY;
    }

    ret = FindStampRecordFile(start, &start_rf);
    if (ret == -ERR_STAMP_TOO_SMALL)
    {
        // use first record file as start record file
        map<UTime, RecordFile*>::iterator iter = record_file_map_.begin();
        assert(iter != record_file_map_.end());
        start_rf = iter->second;
    }
    else if (ret == -ERR_STAMP_TOO_BIG)
    {
        return -ERR_NO_ITEM_FOUND;
    }
    else
    {
        assert(ret == 0);
    }

    RecordFile *record_file = start_rf;
    map<RecordFile*, map<UTime, RecordFile*>::iterator>::iterator search_iter = file_search_map_.find(record_file);
    assert(search_iter != file_search_map_.end());
    map<UTime, RecordFile*>::iterator iter = search_iter->second;
    assert(iter->second == start_rf);
    for (; iter != record_file_map_.end(); iter++)
    {
        record_file = iter->second;
        
        // get all frag info in this record file
        deque<FRAGMENT_INFO_T> all_frag_info;
        ret = record_file->GetAllFragInfo(all_frag_info);
        assert (ret == 0);

        /* start time landed in the record file */
        if (record_file->start_time_ <= start && record_file->end_time_ >= start)
        {
            /* end time landed in the record file */
            if (record_file->start_time_ <= end && record_file->end_time_ >= end)
            {
                ret = SelectFragInfoWithStartAndEndTime(all_frag_info, start, end, frag_info_queue);
                assert(ret == 0);
                break;
            }

            ret = SelectFragInfoWithStartTime(all_frag_info, start, frag_info_queue);
            assert(ret == 0);
            continue;
        }

        /* end time landed in the record file */
        if (record_file->start_time_ <= end && record_file->end_time_ >= end)
        {
            ret = SelectFragInfoWithEndTime(all_frag_info, end, frag_info_queue);
            assert(ret == 0);
            break;
        }

        /* obtain all fragment info in the record file */
        ret = SelectAllFragInfo(all_frag_info, frag_info_queue);
        assert(ret == 0);
    }

    return 0;
}

int32_t RecordFileMap::GetLastRecordFile(RecordFile **record_file)
{
    assert(record_file != NULL);

    RWLock::RDLocker locker(rwlock_);

    map<UTime, RecordFile*>::reverse_iterator riter = record_file_map_.rbegin();
    if (riter == record_file_map_.rend())
    {
        return -ERR_ITEM_NOT_FOUND;
    }

    *record_file = riter->second;
    return 0;
}

int32_t RecordFileMap::PutRecordFile(UTime &time, RecordFile *record_file)
{
    assert(record_file != NULL);

    pair<map<UTime, RecordFile*>::iterator, bool> ret;

    RWLock::WRLocker locker(rwlock_);
    ret = record_file_map_.insert(make_pair(time, record_file));
    if (ret.second == false)
    {
        assert("record file already exist" == 0);
    }

    map<UTime, RecordFile*>::iterator iter = ret.first;
    file_search_map_.insert(make_pair(record_file, iter));

    return 0;
}

int32_t RecordFileMap::OpenWriteRecordFile(RecordFile **record_file)
{
    assert(record_file != NULL);
    int32_t ret;

    RWLock::WRLocker locker(rwlock_);

    map<UTime, RecordFile*>::reverse_iterator riter = record_file_map_.rbegin();
    if (riter == record_file_map_.rend())
    {
        return -ERR_ITEM_NOT_FOUND;
    }

    *record_file = riter->second;
    
    ret = (*record_file)->OpenFd(true);
    assert(ret == 0);

    return 0;
}

int32_t RecordFileMap::AllocWriteRecordFile(UTime &stamp, RecordFile **record_file)
{
    assert(record_file != NULL);
    int32_t ret;
    RecordFile *temp_file = NULL;
    pair<map<UTime, RecordFile*>::iterator, bool> map_ret;

    RWLock::WRLocker lock(rwlock_);

    ret = store_client_->GetFreeFile(&temp_file);
    assert(ret == 0);

    temp_file->start_time_ = stamp;

    map_ret = record_file_map_.insert(make_pair(stamp, temp_file));
    if (map_ret.second == false)
    {
        assert("record file already exist" == 0);
    }

    map<UTime, RecordFile*>::iterator iter = map_ret.first;
    file_search_map_.insert(make_pair(temp_file, iter));

    ret = (*record_file)->OpenFd(true);
    assert(ret == 0);

    return 0;
}

int32_t RecordFileMap::FinishWriteRecordFile(RecordFile *record_file)
{
    assert(record_file != NULL);
    int32_t ret;

    RWLock::WRLocker lock(rwlock_);

    ret = record_file->FinishWrite();
    assert(ret == 0);

    return 0;
}

int32_t RecordFileMap::EraseRecordFile(RecordFile *record_file)
{
    assert(record_file != NULL);

    RWLock::WRLocker locker(rwlock_);

    bool can_recycle = record_file->CanRecycle();
    if (!can_recycle)
    {
        return -ERR_RECORD_FILE_BUSY;
    }

    map<RecordFile*, map<UTime, RecordFile *>::iterator>::iterator search_iter = file_search_map_.find(record_file);
    assert(search_iter != file_search_map_.end());

    map<UTime, RecordFile*>::iterator iter = search_iter->second;
    record_file_map_.erase(iter);
    file_search_map_.erase(search_iter);

    return 0;
}

int32_t RecordFileMap::SeekStampOffset(UTime &stamp, RecordFile **record_file, uint32_t &seek_start_offset, uint32_t &seek_end_offset)
{
    assert(record_file != NULL);
    int32_t ret;

    RWLock::WRLocker lock(rwlock_);

    ret = FindStampRecordFile(stamp, record_file);
    if (ret != 0)
    {
        Log(logger_, "not find stamp %d.%d, ret is %d", stamp.tv_sec, stamp.tv_nsec, ret);
        return ret;
    }

    ret = (*record_file)->SeekStampOffset(stamp, seek_start_offset, seek_end_offset);
    if (ret != 0)
    {
        Log(logger_, "seek stamp %d.%d offset in record file error, ret is %d", stamp.tv_sec, stamp.tv_nsec, ret);
        return ret;
    }

    return 0;
}

int32_t RecordFileMap::FinishReadRecordFile(RecordFile *record_file)
{
    assert(record_file != NULL);
    int32_t ret;

    RWLock::WRLocker lock(rwlock_);

    ret = record_file->FinishRead();
    assert(ret == 0);

    return ret;
}

void RecordFileMap::Shutdown()
{
    Log(logger_, "shutdown");
    
    RWLock::WRLocker lock(rwlock_);
    while(!file_search_map_.empty())
    {
        map<RecordFile*, map<UTime, RecordFile*>::iterator>::iterator p = file_search_map_.begin();

        delete p->first;
        record_file_map_.erase(p->second);
        file_search_map_.erase(p);
    }

    return;
}

// ======================================================= //
//                  record writer
// ======================================================= //
RecordWriter::RecordWriter(Logger *logger,RecordFileMap *file_map)
: logger_(logger), file_map_(file_map), queue_mutex_("RecordWriter::queue_mutex"),
record_file_mutex_("RecordWriter::record_file_mutex"), record_file_(NULL), buffer_(NULL), buffer_write_offset_(0),
current_o_frame_(NULL), write_index_event_(NULL), stop_(false)
{
    memset((void *)&buffer_times_, 0, sizeof(BufferTimes));
}

int32_t RecordWriter::Enqueue(WriteOp *write_op)
{
    assert(write_op != NULL);

    Mutex::Locker lock(queue_mutex_);
    if (stop_)
    {
        Log(logger_, "have stopped, so should't go here");
        return -ERR_SHUTDOWN;
    }

    write_op_queue_.push_back(write_op);
    queue_cond_.Signal();

    return 0;
}

int32_t RecordWriter::Dequeue(WriteOp **write_op)
{
    assert(write_op != NULL);

    *write_op = write_op_queue_.front();
    write_op_queue_.pop_front();

    return 0;
}

int32_t RecordWriter::EncodeFrameHeader(char *header, FRAME_INFO_T *frame)
{
    assert(header != NULL && frame != NULL);
    char *temp = header;

    EncodeFixed32(temp, kMagicCode);
    temp += 4;

    EncodeFixed32(temp, frame->type);
    temp += 4;

    EncodeFixed32(temp, frame->frame_time.seconds);
    temp += 4;

    EncodeFixed32(temp, frame->frame_time.nseconds);
    temp += 4;

    EncodeFixed64(temp, frame->stamp);
    temp += 8;

    EncodeFixed32(temp, frame->size);
    temp += 4;

    return 0;
}

int32_t RecordWriter::EncodeFrame(bool add_o_frame, FRAME_INFO_T *frame, char *temp_buffer)
{
    assert(frame != NULL);

    uint32_t write_offset = 0;
    if (add_o_frame)
    {
        char header[kHeaderSize] = {0}; 
        EncodeFrameHeader(header, current_o_frame_); 

        memcpy(temp_buffer, header, kHeaderSize);
        write_offset += kHeaderSize;
        memcpy(temp_buffer + write_offset, current_o_frame_->buffer, current_o_frame_->size);
        write_offset += current_o_frame_->size;
    }

    char header[kHeaderSize] = {0}; 
    EncodeFrameHeader(header, frame); 

    memcpy(temp_buffer + write_offset, header, kHeaderSize);
    write_offset += kHeaderSize;
    memcpy(temp_buffer + write_offset, frame->buffer, frame->size);
    write_offset += frame->size;

    return 0;
}

int32_t RecordWriter::UpdateBufferTimes(uint32_t type, UTime &time)
{
    if (buffer_times_.start_time == 0)
    {
        buffer_times_.start_time = time;
    }

    if ((buffer_times_.i_frame_start_time == 0) && (type == JVN_DATA_I))
    {
        buffer_times_.i_frame_start_time = time;
    }

    if (buffer_times_.end_time < time)
    {
        buffer_times_.end_time = time;
    }

    if ((buffer_times_.i_frame_end_time < time) && (type == JVN_DATA_I))
    {
        buffer_times_.i_frame_end_time = time;
    }

    return 0;
}

int32_t RecordWriter::WriteBuffer(uint32_t write_length)
{
    assert(record_file_ != NULL);
    int32_t ret;

    ret = record_file_->Append(buffer_, write_length, buffer_times_);
    assert(ret == 0);

    memset(buffer_, 0, kBlockSize);
    buffer_write_offset_ = 0;
    memset(&buffer_times_, 0, sizeof(BufferTimes));

    return 0;
}

int32_t RecordWriter::WriteRecordFileIndex(int r)
{
    int32_t ret;
    struct RecordFileInfo record_file_info_buffer = {0};
    struct RecordFragmentInfo record_frag_info_buffer = {0};
    uint32_t record_frag_info_number = 0;
    IndexFile *index_file = NULL;
    uint32_t file_info_write_offset;
    uint32_t frag_info_write_offset;
    uint32_t file_counts;
    string base_name;
    uint32_t number = 0;

    {
        // build index data
        Mutex::Locker lock(record_file_mutex_);

        if (record_file_ == NULL)
        {
            goto again;
        }

        ret = record_file_->BuildIndex((char *)&record_file_info_buffer, sizeof(struct RecordFileInfo), 
                                        (char *)&record_frag_info_buffer, sizeof(struct RecordFragmentInfo), &record_frag_info_number);
        if (ret == -ERR_RECORD_NO_WRITE)
        {
            goto again;
        }
        assert(ret == 0);

        base_name = record_file_->base_name_;
        number = record_file_->number_;

        ret = index_file_manager->Find(base_name, &index_file);
        assert(ret == 0 && index_file != NULL);
        file_counts = index_file->GetFileCounts();

        file_info_write_offset = number * sizeof(RecordFileInfo);
        frag_info_write_offset = file_counts * sizeof(RecordFileInfo) + 
                                 number * kStripeCount * sizeof(RecordFragmentInfo) + 
                                 (record_frag_info_number -1) * sizeof(RecordFragmentInfo);
        index_file->Write(frag_info_write_offset, (char *)&record_frag_info_buffer, sizeof(struct RecordFragmentInfo));
        index_file->Write(file_info_write_offset, (char *)&record_file_info_buffer, sizeof(struct RecordFileInfo));
    }

again:

    // stopped
    if (r == -1)
    {
        return 0;
    }

    write_index_event_ = new C_WriteIndexTick(this);
    store_client_center->timer.AddEventAfter(WRITE_INDEX_INTERVAL, write_index_event_);

    return 0;
}

int32_t RecordWriter::ResetWriteIndexEvent(uint32_t after_seconds)
{
    Mutex::Locker lock(store_client_center->timer_lock);

    if (write_index_event_ != NULL)
    {
        Log(logger_, "cancel write index, write_index_event_ is %p", write_index_event_);
        store_client_center->timer.CancelEvent(write_index_event_);
        write_index_event_ = NULL;
    }

    write_index_event_ = new C_WriteIndexTick(this);
    store_client_center->timer.AddEventAfter(after_seconds, write_index_event_);
    Log(logger_, "reset write index, write_index_event_ is %p", write_index_event_);

    return 0;
}

int32_t RecordWriter::DoWriteIndexEvent(bool again)
{
    Mutex::Locker lock(store_client_center->timer_lock);

    if (write_index_event_ != NULL)
    {
        store_client_center->timer.DoEvent(write_index_event_);
        write_index_event_ = NULL;
    }
    
    if (again)
    {
        write_index_event_ = new C_WriteIndexTick(this);
        store_client_center->timer.AddEventAfter(WRITE_INDEX_INTERVAL, write_index_event_);
        Log(logger_, "reset write index, write_index_event_ is %p", write_index_event_);
    }

    return 0;
}

void *RecordWriter::Entry()
{
    int32_t ret;
    bool first_i_frame = false;

    queue_mutex_.Lock();

    while(true)
    {
        while(!write_op_queue_.empty())
        {
            WriteOp *write_op = NULL;
            ret = Dequeue(&write_op);
            assert(ret == 0);
            assert(write_op != NULL);
            assert(write_op->frame_info != NULL);

            queue_mutex_.Unlock();

            char *frame_buffer = NULL;
            FRAME_INFO_T *frame = write_op->frame_info;
            safe_free(write_op);

            UTime stamp(frame->frame_time.seconds, frame->frame_time.nseconds);
            uint32_t frame_type = frame->type;
            uint32_t frame_length = kHeaderSize + frame->size;
            uint32_t file_offset = 0;
            bool add_o_frame = false;
            uint32_t file_left_size  = 0;

            if (frame_type == JVN_DATA_O)
            {
                /* swap current o frame and frame */
                FRAME_INFO_T *temp = current_o_frame_;
                current_o_frame_ = frame;
                frame = temp;                
                
                Log(logger_, "O frame, replace current o frame");
                goto FreeResource;
            }
            else if (frame_type == JVN_DATA_I)
            {
                if (current_o_frame_ == NULL)
                {
                    Log(logger_, "I frame, but no O frame");
                    goto FreeResource;
                }

                add_o_frame = true;
                first_i_frame = true;
                current_o_frame_->frame_time.seconds = frame->frame_time.seconds;
                current_o_frame_->frame_time.nseconds = frame->frame_time.nseconds;
                Log(logger_, "I frame, and already have O frame");
            }
            else
            {
                if (first_i_frame == false)
                {
                    Log(logger_, "no I frame, continue");
                    goto FreeResource;
                }
                Log(logger_, "B or P frame");
            }

            if (add_o_frame)
            {
                frame_length += kHeaderSize + current_o_frame_->size;
            }

            {
                Mutex::Locker lock(record_file_mutex_);
                if (record_file_ == NULL)
                {
                    RecordFile *temp_file = NULL;
                    ret = file_map_->OpenWriteRecordFile(&temp_file);
                    if (ret != 0)
                    {
                        ret = file_map_->AllocWriteRecordFile(stamp, &temp_file);
                        assert(ret == 0 && temp_file != NULL);
                    }

                    record_file_ = temp_file;
                }
            }

            file_offset = record_file_->record_offset_;
            assert(file_offset <= kRecordFileSize);
            file_left_size = kRecordFileSize - file_offset;

            frame_buffer = (char *)malloc(frame_length);
            UpdateBufferTimes(frame_type, stamp);
            EncodeFrame(add_o_frame, frame, frame_buffer);

            if ((buffer_write_offset_ + frame_length) <= file_left_size)
            {
                uint32_t temp_buffer_write_offset = 0;
                uint32_t frame_left_len = frame_length;
                while(frame_left_len > 0)
                {
                    uint32_t buffer_left_size = kBlockSize - buffer_write_offset_;
                    if (buffer_left_size == 0)
                    {
                        ret = WriteBuffer(kBlockSize);
                        assert(ret == 0);
                    }

                    uint32_t copy_len = (buffer_left_size >= frame_left_len) ? frame_left_len: buffer_left_size;
                    memcpy(buffer_ + buffer_write_offset_, frame_buffer + temp_buffer_write_offset, copy_len);

                    frame_left_len -= copy_len;
                    buffer_write_offset_ += copy_len;
                    temp_buffer_write_offset += copy_len;
                }

                goto FreeResource;
            }
            else
            {
                /* no enough space in record file */
                if (buffer_write_offset_ != 0)
                {
                    ret = WriteBuffer(kBlockSize);
                    assert(ret == 0);
                }

                DoWriteIndexEvent(true);
                file_map_->FinishWriteRecordFile(record_file_);

                {
                    // alloc new record file
                    Mutex::Locker lock(record_file_mutex_);
                    RecordFile *record_file = NULL;
                    UTime stamp(frame->frame_time.seconds, frame->frame_time.nseconds);
                    ret = file_map_->AllocWriteRecordFile(stamp, &record_file);
                    assert(ret == 0 && record_file != NULL);
                    record_file_ = record_file;
                }

                uint32_t temp_buffer_write_offset = 0;
                uint32_t frame_left_len = frame_length;
                while(frame_left_len > 0)
                {
                    uint32_t buffer_left_size = kBlockSize - buffer_write_offset_;
                    if (buffer_left_size == 0)
                    {
                        ret = WriteBuffer(kBlockSize);
                        assert(ret == 0);
                    }

                    uint32_t copy_len = (buffer_left_size >= frame_left_len) ? frame_left_len : buffer_left_size;
                    memcpy(buffer_ + buffer_write_offset_, frame_buffer + temp_buffer_write_offset, copy_len);

                    frame_left_len -= copy_len;
                    buffer_write_offset_ += copy_len;
                    temp_buffer_write_offset += copy_len;
                }

                goto FreeResource;
            }

FreeResource:    
            if (frame != NULL)
            {
                safe_free(frame->buffer);
            }

            safe_free(frame);
            safe_free(frame_buffer);

            queue_mutex_.Lock();
            continue;
        }
        
        if (stop_)
        {
            if (buffer_write_offset_ != 0 && record_file_ != NULL)
            {
                Log(logger_, "stop_, write left data");

                ret = WriteBuffer(kBlockSize);
                assert(ret == 0);

                DoWriteIndexEvent(false);
                file_map_->FinishWriteRecordFile(record_file_);
            }

            break;
        }

        queue_cond_.Wait(queue_mutex_);
    }

    queue_mutex_.Unlock();
    
    Log(logger_, "entry end");
    return 0;
}

void RecordWriter::Start()
{
    Log(logger_, "Start");
    int32_t ret = 0;

    record_file_ = NULL;
    buffer_ = new char[kBlockSize];
    assert(buffer_ != NULL);
    buffer_write_offset_ = 0;

    stop_ = false;
    Create();

    ResetWriteIndexEvent(WRITE_INDEX_INTERVAL);

    return;
}

void RecordWriter::Stop()
{
    Log(logger_, "stop");

    int32_t ret;

    /* stop queue */
    queue_mutex_.Lock();
    stop_ = true;
    queue_cond_.Signal();
    queue_mutex_.Unlock();
    Join();

    /* delete buffer */
    delete []buffer_;
    buffer_ = NULL;
    buffer_write_offset_ = 0;

    /* free current o frame */
    if (current_o_frame_ != NULL)
    {
        safe_free(current_o_frame_->buffer);
    }
    safe_free(current_o_frame_);

    record_file_ = NULL;

    return;
}

void RecordWriter::Shutdown()
{
    // TODO complete it
    return ;
}

// ======================================================= //
//                  record reader
// ======================================================= //
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
            Log(logger_, "read frame error, read_offset %d, ret %d", read_offset_, ret);
            return ret;
        }

        if (frame->type == JVN_DATA_O)
        {
           /* o frame no change */
           if ((current_o_frame_.type == JVN_DATA_O)
               && (current_o_frame_.size == frame->size)
               && (memcmp(current_o_frame_.buffer, frame->buffer, frame->size)) == 0)
           {
               Log(logger_, "o frame not changed, so skip this o frame");
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
            Log(logger_, "o frame changed");
        }

        break;
    }

    if (read_offset_ >= read_end_offset_)
    {
        Log(logger_, "read offset %d, read end offset %d", read_offset_, read_end_offset_);
        return -ERR_READ_OVER_THAN_END_OFFSET;
    }

    /* return frame */
    read_offset_ += kHeaderSize;
    read_offset_ += frame->size;

    return 0;
}

int32_t RecordReader::Close()
{
    safe_free(current_o_frame_.buffer);

    if (record_file_ != NULL)
    {
        Log(logger, "finished to read record file, %srecord_%05d, current read offset %d, read end offset %d", 
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

// ======================================================= //
//                  store client
// ======================================================= //
StoreClient::StoreClient(Logger *logger, string stream_info)
: logger_(logger), stream_info_(stream_info), record_file_map_(logger_), 
 writer(logger_, &record_file_map_), reader_mutex_("StoreClient::read_mutex_")
{

}

bool StoreClient::IsRecordFileEmpty()
{
    return record_file_map_.Empty();
}

string StoreClient::GetStreamInfo()
{
    return stream_info_;
}

int32_t StoreClient::OpenWrite(int32_t id)
{
    Log(logger_, "open write, id is %d", id);

    writer.Start();
    
    return 0;
}

int32_t StoreClient::EnqueueFrame(FRAME_INFO_T *frame)
{
    assert(frame != NULL);
    assert(frame->buffer != NULL);
    //Log(logger_, "enqueue frame %p", frame);

    int32_t ret;

    WriteOp *write_op = (WriteOp*)malloc(sizeof(WriteOp));
    assert(write_op != NULL);

    write_op->frame_info = (FRAME_INFO_T *)malloc(sizeof(FRAME_INFO_T));
    assert(write_op->frame_info != NULL);

    write_op->frame_info->type = frame->type;
    write_op->frame_info->frame_time = frame->frame_time;
    write_op->frame_info->stamp = frame->stamp;
    write_op->frame_info->size = frame->size;
    if (frame->size != 0)
    {
        write_op->frame_info->buffer = (char *)malloc(frame->size);
        assert(write_op->frame_info != NULL);
        memcpy(write_op->frame_info->buffer, frame->buffer, frame->size);
    }

    ret = writer.Enqueue(write_op);

    return ret;
}

int32_t StoreClient::CloseWrite(int32_t id)
{
    Log(logger_, "close write id %d", id);
    
    writer.Stop();

    free_file_table->Close(stream_info_);

    return 0;
}

int32_t StoreClient::OpenRead(int32_t id)
{
    Log(logger_, "open read, id is %d", id);
    pair<map<int32_t, RecordReader*>::iterator, bool> ret;

    Mutex::Locker lock(reader_mutex_);
    RecordReader *record_reader = new RecordReader(logger_, &record_file_map_);
    ret = record_readers_.insert(make_pair(id, record_reader));
    assert(ret.second == true);

    return 0;
}

int32_t StoreClient::SeekRead(int32_t id, UTime &stamp)
{
    int32_t ret;
    RecordReader *record_reader = NULL;
    Log(logger_, "seek read id, id is %d", id);

    {
        Mutex::Locker lock(reader_mutex_);
        map<int32_t, RecordReader*>::iterator iter = record_readers_.find(id);
        if (iter == record_readers_.end())
        {
            return -ERR_ITEM_NOT_FOUND;
        }
        
        record_reader = iter->second;
        assert(record_reader != NULL);
    }

    ret = record_reader->Seek(stamp);
    assert(ret == 0);

    return 0;
}

int32_t StoreClient::ReadFrame(int32_t id, FRAME_INFO_T *frame)
{
    assert(frame != NULL);
    Log(logger_, "read frame, id is %d", id);

    int32_t ret = 0;
    RecordReader *record_reader = NULL;

    {
        Mutex::Locker lock(reader_mutex_);
        map<int32_t, RecordReader*>::iterator iter = record_readers_.find(id);
        if (iter == record_readers_.end())
        {
            return -ERR_ITEM_NOT_FOUND;
        }
        record_reader = iter->second;
    }

    ret = record_reader->ReadFrame(frame);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

int32_t StoreClient::GetFreeFile(RecordFile **record_file)
{
    assert(record_file != NULL);

    int32_t ret;
    RecordFile *free_file = NULL;

    ret = free_file_table->Get(stream_info_, &free_file);
    assert(ret == 0 && free_file != NULL);

    free_file->stream_info_ = stream_info_;

    /* used to recycle */
    ret = store_client_center->AddToRecycleQueue(this, free_file);
    assert(ret == 0);

    *record_file = free_file;
    return 0;
}

int32_t StoreClient::GetLastRecordFile(RecordFile **record_file)
{
    assert(record_file != NULL);
    Log(logger_, "get last record file");

    return record_file_map_.GetLastRecordFile(record_file);
}

int32_t StoreClient::SeekReaderStampOffset(UTime &stamp, RecordFile **record_file, uint32_t &seek_start_offset, uint32_t &seek_end_offset)
{
    return record_file_map_.SeekStampOffset(stamp, record_file, seek_start_offset, seek_end_offset);
}

int32_t StoreClient::PutRecordFile(UTime &stamp, RecordFile *record_file)
{
    Log(logger_, "put record file, stamp is %d.%d", stamp.tv_sec, stamp.tv_nsec);

    return record_file_map_.PutRecordFile(stamp, record_file);
}

int32_t StoreClient::RecycleRecordFile(RecordFile *record_file)
{
    Log(logger_, "recycle record file, record file is %p");
    record_file_map_.EraseRecordFile(record_file);

    return 0;
}

int32_t StoreClient::CloseRead(int32_t id)
{
    Log(logger_, "close read id %d");

    map<int32_t, RecordReader*>::iterator iter = record_readers_.find(id);
    if (iter == record_readers_.end())
    {
        return -ERR_ITEM_NOT_FOUND;
    }

    RecordReader *record_reader = iter->second;
    record_reader->Close();
    delete record_reader;

    return 0;
}

int32_t StoreClient::ListRecordFragments(UTime &start, UTime &end, deque<FRAGMENT_INFO_T> &frag_info_queue)
{
    Log(logger_, "list record fragments, start is %d.%d, end is %d.%d",
        start.tv_sec, start.tv_nsec, end.tv_sec, end.tv_nsec);
    
    int32_t ret;

    ret = record_file_map_.ListRecordFragments(start, end, frag_info_queue);
    if (ret != 0)
    {
        Log(logger_, "list record fragments return %d", ret);
        return ret;
    }

    return 0;
}

void StoreClient::Shutdown()
{
    writer.Shutdown();
    record_file_map_.Shutdown();

    //TODO reader shutdown
    return;
}

// ======================================================= //
//              store client center 
// ======================================================= //
StoreClientCenter::StoreClientCenter(Logger *logger)
: logger_(logger), rwlock_("StoreClientCenter::RWLock"), recycle_mutex_("StoreClientCenter::RecycleMutex"),
timer_lock("StoreClientCenter::timer_lock"), timer(logger_, timer_lock)
{
    clients_.resize(MAX_STREAM_COUNTS, 0);
}

int32_t StoreClientCenter::Init()
{
    {
        Mutex::Locker lock(timer_lock);
        timer.Init();
    }

    return 0;
}

int32_t StoreClientCenter::Open(int flag, int32_t id, string &stream_info)
{
    assert(flag == 0 || flag == 1);
    Log(logger_, "open store client, flag is %d, id is %d, stream info is %s", 
        flag, id, stream_info.c_str());

    int32_t ret;
    StoreClient *client = NULL;

    if (flag == 0)
    {
        ret = FindStoreClient(stream_info, &client);
        if (ret != 0)
        {
            return -ERR_ITEM_NOT_FOUND;
        }

        {
            RWLock::WRLocker lock(rwlock_);
            clients_[id] = client;
        }

        ret = client->OpenRead(id);
    }
    else
    {
        ret = FindStoreClient(stream_info, &client);
        if (ret != 0)
        {
            client = new StoreClient(logger_, stream_info);
            assert(client != NULL);

            {
                RWLock::WRLocker lock(rwlock_);
                clients_[id] = client;
                client_search_map_.insert(make_pair(stream_info, client));
            }
        }
        else
        {
            RWLock::WRLocker lock(rwlock_);
            clients_[id] = client;
        }

        ret = client->OpenWrite(id);
    }

    Log(logger_, "open store client, flag is %d, id is %d, ret is %d", flag, id, ret);

    return ret;
}

int32_t StoreClientCenter::AddStoreClient(string &stream_info, StoreClient **client)
{
    *client = new StoreClient(logger_, stream_info);
    assert(*client != NULL);

    RWLock::WRLocker lock(rwlock_);
    client_search_map_.insert(make_pair(stream_info, *client));

    return 0;
}

int32_t StoreClientCenter::GetStoreClient(int32_t id, StoreClient **client)
{
    assert(client != NULL);
    Log(logger_, "get store client, id is %d", id);

    if (id >= MAX_STREAM_COUNTS)
    {
        Log(logger_, "id %d exceed max stream id", id);
        return -ERR_ITEM_NOT_FOUND;
    }

    RWLock::RDLocker lock(rwlock_);
    *client = clients_[id];
    if (*client == NULL)
    {
        return -ERR_ITEM_NOT_FOUND;
    }

    return 0;
}

int32_t StoreClientCenter::FindStoreClient(string stream_info, StoreClient **client)
{
    assert(client != NULL);

    Log(logger_, "find store client, stream info is %s", stream_info.c_str());

    RWLock::RDLocker lock(rwlock_);
    map<string, StoreClient*>::iterator iter = client_search_map_.find(stream_info);
    if (iter == client_search_map_.end())
    {
        return -ERR_ITEM_NOT_FOUND;
    }

    *client = iter->second;

    return 0;
}

int32_t StoreClientCenter::RemoveStoreClient(StoreClient *client)
{
    assert(client != NULL);
    Log(logger_, "remove store client, client is %p", client);
    
    RWLock::WRLocker lock(rwlock_);

    string stream_info = client->GetStreamInfo();
    map<string, StoreClient*>::iterator map_iter = client_search_map_.find(stream_info);
    assert(map_iter != client_search_map_.end());
    client_search_map_.erase(map_iter);

    client->Shutdown();
    delete client;
    client = NULL;

    return 0;
}

int32_t StoreClientCenter::Close(int32_t id, int flag)
{
    assert(flag == 0 || flag == 1);
    Log(logger_, "close store client %d, flag is %d", id, flag);

    int32_t ret;
    StoreClient *client = NULL;

    ret = GetStoreClient(id, &client);
    if (ret != 0)
    {
        Log(logger_, "get store client %d error, ret is %d", id, ret);
        return ret;
    }

    if (flag == 0)
    {
        ret = client->CloseRead(id);
    }
    else
    {
        ret = client->CloseWrite(id);

    }

    {
        RWLock::WRLocker lock(rwlock_);
        clients_[id] = NULL;
    }

    Log(logger_, "close store client %d, return ret %d", id, ret);

    return ret;
}

int32_t StoreClientCenter::WriteFrame(int32_t id, FRAME_INFO_T *frame)
{
    assert(frame != NULL);
    Log(logger_, "write frame, id is %d, frame %p, buffer size is %d", id, frame, frame->size);

    int32_t ret;
    StoreClient *client = NULL;

    ret = GetStoreClient(id, &client);
    if (ret != 0)
    {
        return ret;
    }

    return client->EnqueueFrame(frame);
}

int32_t StoreClientCenter::SeekRead(int32_t id, UTime &stamp)
{
    Log(logger_, "seek read, id is %d, stamp is %d.%d", id, stamp.tv_sec, stamp.tv_nsec);

    int32_t ret;
    StoreClient *client = NULL;

    ret = GetStoreClient(id, &client);
    if (ret != 0)
    {
        return ret;
    }

    return client->SeekRead(id, stamp);
}

int32_t StoreClientCenter::ReadFrame(int32_t id, FRAME_INFO_T *frame)
{
    assert(frame != NULL);

    int32_t ret;
    StoreClient *client = NULL;

    ret = GetStoreClient(id, &client);
    if (ret != 0)
    {
        return ret;
    }

    return client->ReadFrame(id, frame);
}

int32_t StoreClientCenter::ListRecordFragments(int32_t id, UTime &start, UTime &end, deque<FRAGMENT_INFO_T> &frag_info_queue)
{
    Log(logger_, "list record fragments, id is %d, start time is %d.%d, end time is %d.%d", 
        id, start.tv_sec, start.tv_nsec, end.tv_sec, end.tv_nsec);

    int32_t ret;
    StoreClient *store_client = NULL;

    ret = GetStoreClient(id, &store_client);
    if (ret != 0)
    {
        Log(logger_, "get store client return %d", ret);
        return ret;
    }

    return store_client->ListRecordFragments(start, end, frag_info_queue);
}

int32_t StoreClientCenter::AddToRecycleQueue(StoreClient *store_client, RecordFile *record_file)
{
    assert(store_client != NULL);
    assert(record_file != NULL);
    Log(logger_, "add to recycle queue, store_client is %p, record_file is %p", store_client, record_file);

    Mutex::Locker lock(recycle_mutex_);
    RecycleItem recycle_item;
    recycle_item.store_client = store_client;
    recycle_item.record_file = record_file;

    recycle_queue_.push_back(recycle_item);

    return 0;
}

int32_t StoreClientCenter::StartRecycle()
{
    Log(logger_, "start to recycle");

    Mutex::Locker lock(timer_lock);
    C_Recycle *recycle_event = new C_Recycle(this);
    assert(recycle_event != NULL);
    timer.AddEventAfter(0, recycle_event);

    return 0;
}

int32_t StoreClientCenter::Recycle()
{
    uint32_t recycle_count = 0;
    int32_t ret = 0;
    Log(logger_, "recycle");

    Mutex::Locker lock(recycle_mutex_);
    deque<RecycleItem>::iterator iter = recycle_queue_.begin();
    while(iter != recycle_queue_.end() && recycle_count <= kFilesPerRecycle)
    {
        RecycleItem recycle_item = *iter;
        RecordFile *record_file = recycle_item.record_file;
        StoreClient *store_client = recycle_item.store_client;

        assert(record_file != NULL);
        assert(store_client != NULL);

        ret = store_client->RecycleRecordFile(record_file);
        if (ret == -ERR_RECORD_FILE_BUSY)
        {
            iter++;
            continue;
        }

        assert(ret == 0);

        if (store_client->IsRecordFileEmpty())
        {
            ret = store_client_center->RemoveStoreClient(store_client);
            assert(ret == 0);
        }

        deque<RecycleItem>::iterator del_iter = iter;
        iter++;
        recycle_queue_.erase(del_iter);

        /* add to free file table */
        free_file_table->Put(record_file);
        recycle_count++;
    }

    if (iter == recycle_queue_.end())
    {
        Log(logger_, "recycle reached to end of recycle queue");
    }

    return 0;
}

void StoreClientCenter::Shutdown()
{
    {
        RWLock::WRLocker lock(rwlock_);
        map<string, StoreClient*>::iterator iter = client_search_map_.begin();
        for(; iter != client_search_map_.end(); iter++)
        {
            StoreClient *store_client = iter->second;
            assert(store_client != NULL);
            store_client->Shutdown();
            delete store_client;
            iter->second = NULL;
        }
    }

    {
        Mutex::Locker lock(timer_lock);
        timer.Shutdown();
    }

    return;
}

int32_t StoreClientCenter::DumpClientSearchMap()
{
    RWLock::RDLocker lock(rwlock_);

    map<string, StoreClient*>::iterator iter = client_search_map_.begin();
    for (; iter != client_search_map_.end(); iter++)
    {
        string temp = iter->first;
        StoreClient *store_client = iter->second;
        string stream_info = store_client->GetStreamInfo();
        fprintf(stderr, "stream info is %s, storeclient is %p, stream info of store client is %s\n", temp.c_str(), store_client,
            stream_info.c_str());
    }

    return 0;
}

}

