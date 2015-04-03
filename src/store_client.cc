#include "store_client.h"
#include "storage.h"
#include "record_reader.h"

namespace storage
{

RecordFileMap::RecordFileMap(Logger *logger, StoreClient *store_client)
: logger_(logger), store_client_(store_client), rwlock_("RecordFileMap::rwlock")
{

}

bool RecordFileMap::Empty()
{
    RWLock::RDLocker lock(rwlock_);
    return record_file_map_.empty();
}

int32_t RecordFileMap::GetRecordFileNumbers()
{
    RWLock::RDLocker lock(rwlock_);
    return record_file_map_.size();
}

int32_t RecordFileMap::FindStampInOrLowerRecordFile(UTime &time, RecordFile **record_file)
{
    assert(record_file != NULL);
    //Log(logger_, "find stamp %d.%d", time.tv_sec, time.tv_nsec);

    map<UTime, RecordFile*>::iterator iter_up;

    if (record_file_map_.empty())
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
            *record_file = iter->second;
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
        map<UTime, RecordFile*>::iterator back_iter = iter_up;
        iter_up--;
        RecordFile * temp_file = iter_up->second;

        if (temp_file->start_time_ <= time && time <= temp_file->end_time_)
        {
            *record_file = temp_file;
        }
        else
        {
            *record_file = back_iter->second;
        }
    }

    return 0;
}

int32_t RecordFileMap::SelectFragInfoWithStartTime(deque<FRAGMENT_INFO_T> &all_frag_info, UTime &start, deque<FRAGMENT_INFO_T> &select_frag_info)
{
    Log(logger_, "select frag info with start time %d.%d", start.tv_sec, start.tv_nsec);

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
    Log(logger_, "select frag info with start time %d.%d and end time %d.%d", start.tv_sec, start.tv_nsec, end.tv_sec, end.tv_nsec);

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
    Log(logger_, "select frag info with end time %d.%d", end.tv_sec, end.tv_nsec);

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
    Log(logger_, "select all frag info");

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
    int32_t ret;
    RecordFile *start_rf = NULL;

    if (Empty())
    {
        return -ERR_TABLE_IS_EMPTY;
    }

    RWLock::RDLocker lock(rwlock_);
    ret = FindStampInOrLowerRecordFile(start, &start_rf);
    if (ret == -ERR_STAMP_TOO_SMALL)
    {
        // use first record file as start record file
        map<UTime, RecordFile*>::iterator iter = record_file_map_.begin();
        assert(iter != record_file_map_.end());
        start_rf = iter->second;
        
        /* check end time of this find */
        if (end < start_rf->start_time_)
        {
            LOG_INFO(logger_, "end time %d.%d < first_record_file start time %d.%d", end.tv_sec, end.tv_nsec,
                                start_rf->start_time_.tv_sec, start_rf->start_time_.tv_nsec);
            return -ERR_NO_ITEM_FOUND;
        }
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

        Log(logger_, "record file %srecord_%05d get all frag info, count is %d",
                        record_file->base_name_.c_str(), record_file->number_, all_frag_info.size());
        deque<FRAGMENT_INFO_T>::iterator iter = all_frag_info.begin();
        int count = 0;
        for (; iter != all_frag_info.end(); iter++)
        {
            FRAGMENT_INFO_T temp_frag = *iter;
            Log(logger_, " frag seq %d, %d.%d to %d.%d", count++, 
                        temp_frag.start_time.seconds, temp_frag.start_time.nseconds, temp_frag.end_time.seconds, temp_frag.end_time.nseconds);
        }

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

    ret = store_client_->GetFreeFile(&temp_file);
    assert(ret == 0);
    temp_file->start_time_ = stamp;
    temp_file->end_time_ = stamp;

    {
        RWLock::WRLocker lock(rwlock_);
        map_ret = record_file_map_.insert(make_pair(stamp, temp_file));
        if (map_ret.second == false)
        {
            assert("record file already exist" == 0);
        }
    
        map<UTime, RecordFile*>::iterator iter = map_ret.first;
        file_search_map_.insert(make_pair(temp_file, iter));
    
        ret = temp_file->OpenFd(true);
        assert(ret == 0);
    }

    /* used to recycle */
    ret = store_client_center->AddToRecycleQueue(store_client_, temp_file);
    assert(ret == 0);

    *record_file = temp_file;

    return 0;
}

int32_t RecordFileMap::FinishWriteRecordFile(RecordFile *record_file)
{
    assert(record_file != NULL);
    int32_t ret;

    // update record file in recycle queue
    {
        ret = store_client_center->UpdateRecordFileInRecycleQueue(store_client_, record_file);
        assert(ret == 0);
    }

    // finish write
    {
        RWLock::WRLocker lock(rwlock_);

        ret = record_file->FinishWrite();
        assert(ret == 0);
    }

    LOG_DEBUG(logger_, "finish write record file %srecord_%05d, write offset %d, start time %d.%d, end time %d.%d", record_file->base_name_.c_str(),
        record_file->number_, record_file->record_offset_, record_file->start_time_.tv_sec, record_file->start_time_.tv_nsec,
        record_file->end_time_.tv_sec, record_file->end_time_.tv_nsec);

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

    if (Empty())
    {
        return -ERR_TABLE_IS_EMPTY;
    }

    RWLock::WRLocker lock(rwlock_);
    ret = FindStampInOrLowerRecordFile(stamp, record_file);
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

StoreClient::StoreClient(Logger *logger, string stream_info)
: logger_(logger), stream_info_(stream_info), record_file_map_(logger_, this), 
 writer(logger_, &record_file_map_), reader_mutex_("StoreClient::read_mutex_"), use_count_()
{

}

bool StoreClient::CheckRecycle()
{
    // some one use this client ?
    if (GetUseCount() > 0)
    {
        return false;
    }

    if (!record_file_map_.Empty())
    {
        return false;
    }

    return true;
}

void StoreClient::IncUse()
{
    use_count_.Inc();
    return;
}

void StoreClient::DecUse()
{
    use_count_.Dec();
    return;
}

int32_t StoreClient::GetUseCount()
{
    return use_count_.Get();
}

string StoreClient::GetStreamInfo()
{
    return stream_info_;
}

int32_t StoreClient::GetRecordFileNumbers()
{
    return record_file_map_.GetRecordFileNumbers();
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
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

int32_t StoreClient::ReadFrame(int32_t id, FRAME_INFO_T *frame)
{
    assert(frame != NULL);

    int32_t ret = 0;
    RecordReader *record_reader = NULL;

    {
        Mutex::Locker lock(reader_mutex_);
        map<int32_t, RecordReader*>::iterator iter = record_readers_.find(id);
        if (iter == record_readers_.end())
        {
            LOG_WARN(logger_, "find record reader error, id %d", id);
            return -ERR_ITEM_NOT_FOUND;
        }
        record_reader = iter->second;
    }

    ret = record_reader->ReadFrame(frame);
    if (ret != 0)
    {
        LOG_WARN(logger_, "read frame error, id %d", id);
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

    *record_file = free_file;
    return 0;
}

int32_t StoreClient::GetLastRecordFile(RecordFile **record_file)
{
    assert(record_file != NULL);
    Log(logger_, "get last record file");

    return record_file_map_.GetLastRecordFile(record_file);
}

int32_t StoreClient::PutRecordFile(UTime &stamp, RecordFile *record_file)
{
    Log(logger_, "put record file, stamp is %d.%d", stamp.tv_sec, stamp.tv_nsec);

    record_file_map_.PutRecordFile(stamp, record_file);
    store_client_center->AddToRecycleQueue(this, record_file);

    return 0;
}

int32_t StoreClient::RecycleRecordFile(RecordFile *record_file)
{
    Log(logger_, "recycle record file, record file is %p");
    return record_file_map_.EraseRecordFile(record_file);
}

int32_t StoreClient::CloseRead(int32_t id)
{
    Log(logger_, "close read id %d");

    Mutex::Locker lock(reader_mutex_);
    map<int32_t, RecordReader*>::iterator iter = record_readers_.find(id);
    if (iter == record_readers_.end())
    {
        return -ERR_ITEM_NOT_FOUND;
    }

    RecordReader *record_reader = iter->second;
    record_reader->Close();
    delete record_reader;
    record_readers_.erase(id);

    return 0;
}

int32_t StoreClient::ListRecordFragments(UTime &start, UTime &end, deque<FRAGMENT_INFO_T> &frag_info_queue)
{
    int32_t ret;

    ret = record_file_map_.ListRecordFragments(start, end, frag_info_queue);
    if (ret != 0)
    {
        Log(logger_, "list record fragments return %d", ret);
        return ret;
    }

    return 0;
}

void StoreClient::Dump()
{
    LOG_INFO(logger_, "store client %s, has %d record files, writer stopped: %d, reader number: %d, use_count: %d", 
                stream_info_.c_str(), record_file_map_.GetRecordFileNumbers(), writer.IsStopped(), record_readers_.size(), GetUseCount());
    return;
}

}

