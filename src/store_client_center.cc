#include "store_client_center.h"
#include "../util/coding.h"
#include "index_file.h"
#include "../include/storage.h"

namespace storage
{
// ======================================================= //
//                  record file map
// ======================================================= //
RecordFileMap::RecordFileMap(Logger *logger)
: logger_(logger), rwlock_("RecordFileMap::rwlock"), stop_(false)
{

}

int32_t RecordFileMap::GetRecordFile(UTime time, RecordFile **record_file)
{
    assert(record_file != NULL);
    Log(logger_, "get record file time %d.%d", time.tv_sec, time.tv_nsec);

    map<UTime, RecordFile*>::iterator iter_up;
    map<UTime, RecordFile*>::iterator iter;

    RWLock::RDLocker locker(rwlock_);
    if (stop_)
    {
        return -ERR_SHUTDOWN;
    }

    iter_up = record_file_map_.upper_bound();
    if (iter_up == record_file_map_.end())
    {
        iter = record_file_map_.rbegin();
    }
    else
    {
        iter = iter_up--;
    }

    if (iter == record_file_map_.rend())
    {
        return -ERR_ITEM_NOT_FOUND;
    }

    *record_file = iter->second;
    return 0;
}

int32_t RecordFileMap::GetLastRecordFile(RecordFile **record_file)
{
    assert(record_file != NULL);
    Log(logger_, "get last record file");

    map<UTime, RecordFile*>::iterator iter;

    RWLock::RDLocker locker(rwlock_);
    if (stop_)
    {
        return -ERR_SHUTDOWN;
    }

    iter = record_file_map_.rbegin();
    if (iter == record_file_map_.rend())
    {
        return -ERR_ITEM_NOT_FOUND;
    }

    *record_file = iter->second;
    return 0;
}

int32_t RecordFileMap::PushBackRecordFile(UTime time, RecordFile *record_file)
{
    assert(record_file != NULL);
    Log(logger_, "push back record file, time %d.%d", time.tv_sec, time.tv_nsec);

    pair<map<UTime, RecordFile*>::iterator, bool> ret;

    RWLock::WRLocker locker(rwlock_);
    if (stop_)
    {
        return -ERR_SHUTDOWN;
    }

    ret = record_file_map_.insert(make_pair(time, record_file));
    if (ret->second == false)
    {
        assert("record file already exist" == 0);
    }

    return 0;
}

void RecordFileMap::Shutdown()
{
    Log(logger_, "shutdown");
    RWLock::WRLocker locker(rwlock_);
    stop_ = true;

    map<UTime, RecordFile*>::iterator iter = record_file_map_.begin();
    for (iter; iter != record_file_map_.end(); iter++)
    {
        delete iter->second;
    }

    return;
}

// ======================================================= //
//                  store client writer
// ======================================================= //
RecordWriter::Writer(Logger *logger, StoreClient *store_client)
: logger_(logger), store_client_(store_client), queue_mutex_("RecordWriter::locker"), current_o_frame_(NULL), 
write_index_event_mutex_("RecordWriter::write_index_event_mutex_"), write_index_event(NULL), 
last_record_file_mutex_("RecordWriter::Last_record_file_mutex"), stop_(false)
{
    memset(buffer_times_, 0, sizeof(BufferTimes));
}

int32_t RecordWriter::Enqueue(WriteOp *write_op)
{
    assert(write_op != NULL);
    Log(logger_, "writer enqueue write_op %p", write_op);

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
    Log(logger_, "writer dequeue write_op");

    Mutex::Locker locker(queue_mutex_);

    *write_op = write_op_queue_.front();
    write_op_queue_.pop_front();

    return 0;
}

int32_t RecordWriter::EncodeHeader(char *header, FRAME_INFO_T *frame)
{
    assert(header != NULL && frame != NULL);
    Log(logger_, "encode header, header is %p, frame is %p", header, frame);

    EncodeFixed32(header, kMagicCode);
    EncodeFixed32(header, frame->type);
    EncodeFixed32(header, frame->frame_time.seconds);
    EncodeFixed32(header, frame->frame_time.nseconds);
    EncodeFixed64(header, frame->stamp);
    EncodeFixed32(header, frame->size);

    return 0;
}

int32_t RecordWriter::EncodeFrame(bool add_o_frame, FRAME_INFO_T *frame)
{
    assert(frame != NULL);
    Log(logger_, "encode frame, add_o_frame is %d, frame is %p", add_o_frame, frame);

    if (add_o_frame)
    {
        char header[kHeaderSize] = {0}; 
        EncodeHeader(header, current_o_frame_); 
        buffer_.append(header, kHeaderSize); 
        buffer_.append(current_o_frame_->buffer, current_o_frame_->size);
    }

    char header[kHeaderSize] = {0}; 
    EncodeHeader(header, frame); 
    buffer_.append(header, kHeaderSize); 
    buffer_.append(frame->buffer, frame->size); 

    return 0;
}

int32_t RecordWriter::UpdateBufferTimes(uint32_t type, UTime time);
{
    Log(logger_, "update buffer times");

    if (buffer_times_.start_time == 0)
    {
        buffer_times_.start_time = time;
    }

    if ((buffer_times_.i_frame_start_time == 0) && (type == JVN_DATA_I))
    {
        buffer_times_.i_frame_start_time = time;
    }

    if (buffer_times_.end_time < update.end_time)
    {
        buffer_times_.end_time = time;
    }

    if ((buffer_times_.i_frame_end_time < time) && (type == JVN_DATA_I))
    {
        buffer_times_.i_frame_end_time = time;
    }

    return 0;
}

int32_t RecordWriter::WriteBuffer(RecordFile *record_file, uint32_t write_length)
{
    assert(record_file != NULL);
    Log(logger_, "write buffer, write_length is %d, record file base name is %s, number is %d", 
            write_length, record_file->base_name_, record_file->number_);

    Mutex::Locker lock(last_record_file_mutex_);

    ret = reocrd_file->Append(buffer_, write_length, buffer_times_);
    assert(ret == 0);

    buffer_.erase(0, write_length);
    memset(&buffer_times_, 0, sizeof(BufferTimes));

    return 0;
}

int32_t RecordWriter::BuildRecordFileIndex(RecordFile *file, char *record_file_info_buffer, uint32_t record_file_info_length
                            char *record_frag_info_buffer, uint32_t record_frag_info_length, uint32_t *record_flag_info_number)
{
    assert(record_file != NULL);
    Log(logger_, "build record file index");

    RecordFile *record_file = NULL;
    if (file == NULL)
    {
        ret = store_client_->GetLastRecordFile(&record_file);
        assert(ret == 0 && record_file != NULL);
    }
    else
    {
        record_file = file;
    }

    Mutex::Locker lock(last_record_file_mutex_);

    return record_file->BuildIndex(record_file_info_buffer, record_file_info_length, 
                                    record_frag_info_buffer, record_frag_info_length, record_flag_info_number);
}

int32_t RecordWriter::WriteRecordFileIndex(RecordFile *record_file, int r)
{
    assert(write_index_event != NULL);
    Log(logger_, "write index, write_index_event is %p", write_index_event);
    write_index_event = NULL;

    int32_t ret;
    struct RecordFileInfo record_file_info_buffer = {0}:
    struct RecordFragmentInfo record_frag_info_buffer = {0};
    uint32_t record_frag_info_number = 0;

    ret = BuildRecordFileIndex(record_file, (char *)&record_file_info_buffer, sizeof(struct RecordFileInfo), 
                                (char *)&record_frag_info_buffer, sizeof(struct RecordFragmentInfo), &record_frag_info_number);
    if (ret == -ERR_RECORD_WRITE_OFFSET_ZERO)
    {
        goto Again;
    }
    else
    {
        assert(ret == 0);
    }

    IndexFile *index_file = NULL;
    ret = index_file_manager->Find(record_file->base_name_, &index_file);
    assert(ret == 0 && index_file != NULL);

    uint32_t file_info_write_offset = record_file->number_ * sizeof(RecordFileInfo);
    uint32_t frag_info_write_offset = index_file->file_counts_ * sizeof(RecordFileInfo) + 
            record_file->number_ * kStripeCount * sizeof(RecordFragmentInfo) + record_frag_info_number * sizeof(RecordFragmentInfo);
    index_file->Write(file_info_write_offset, record_file_info_buffer, record_file_info_length);
    index_file->Write(frag_info_write_offset, record_frag_info_buffer, record_frag_info_length);

    if (r == -1)
    {
        return 0;
    }

Again:
    write_index_event = new C_WriteIndexTick(store_client_, NULL);
    store_client_center->timer.AddEventAfter(WRITE_INDEX_INTERVAL, write_index_event);

    return 0;
}

int32_t RecordWriter::ResetWriteIndexEvent(RecordFile *record_file, uint32_t after_seconds)
{
    Mutex::Locker lock(store_client_center->timer_lock);

    if (write_index_event != NULL)
    {
        Log(logger_, "cancel write index, write_index_event is %p", write_index_event);
        store_client_center->timer.CancelEvent(write_index_event);
        write_index_event = NULL;
    }

    write_index_event = new C_WriteIndexTick(store_client_, record_file);
    store_client_center->timer.AddEventAfter(after_seconds, write_index_event);
    Log(logger_, "reset write index, write_index_event is %p, record_file is %p", write_index_event, record_file);

    return 0;
}

int32_t RecordWriter::Entry()
{
    int32_t ret;
    Log(logger_, "entry");

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

            FRAME_INFO_T *frame = write_op->frame_info;
            safe_free(write_op);

            uint32_t frame_type = frame->type;
            bool add_o_frame = false;
            if (frame_type == JVN_DATA_O)
            {
                if (current_o_frame_ != NULL)
                {
                    safe_free(current_o_frame_->buffer);
                    safe_free(current_o_frame_);
                }

                current_o_frame_ = frame;
                queue_mutex_.Lock();
                continue;
            }
            else
            {
                if (current_o_frame_ == NULL)
                {
                    safe_free(frame->buffer);
                    safe_free(frame);
                    queue_mutex_.Lock();
                    continue;
                }

                if (frame_type == JVN_DATA_I)
                {
                    add_o_frame = true;
                }
            }

            uint32_t buffer_old_length = buffer_->length();
            uint32_t frame_length = kHeaderSize + frame->size;
            if (add_o_frame)
            {
                frame_length += kHeaderSize + current_o_frame_->size;
            }

            RecordFile *record_file = NULL;
            ret = store_client_->GetLastRecordFile(&record_file);
            if (ret == ERR_ITEM_NOT_FOUND)
            {
                Log(logger_, "doesn't find any record file");
                ret = store_client_->GetFreeFile(frame->frame_time, &record_file);
                assert(ret == 0);
                assert(record_file != NULL);
            }

            uint32_t write_offset = record_file->record_offset_;
            assert(write_length <= kRecordFileSize);
            uint32_t file_left_size = kRecordFileSize - write_offset;

            if ((buffer_old_length + frame_length) <= file_left_size)
            {
                EncodeFrame(add_o_frame, frame);
                
                /* 更新当前buffer的时间 */
                UpdateBufferTimes(type, frame->frame_time);

                uint32_t buffer_length = buffer.length();
                if (buffer_length >= kBlockSize)
                {
                    uint32_t block_count = buffer_length / kBlockSize;
                    uint32_t write_length = block_count * kBlockSize;
                    ret = WriteBuffer(record_file, write_length);
                    assert(ret == 0);
                }
            }
            else
            {
                /* no enough space in record file */
                if (buffer_old_length != 0)
                {
                    uint32_t write_length = buffer_old_length;
                    ret = WriteBuffer(record_file, write_length);
                    assert(ret == 0);
                }

                record_file->FinishWrite();
                ResetWriteIndexEvent(record_file, 0);

                EncodeFrame(add_o_frame, frame);
                UpdateBufferTimes(type, frame->frame_time);

                RecordFile *record_file = NULL;
                ret = store_client_->GetFreeFile(frame->frame_time, &record_file);
                assert(ret == 0 && record_file != NULL);

                uint32_t buffer_length = buffer_.length();
                if (buffer_length >= kBlockSize)
                {
                    uint32_t block_count = buffer_length / kBlockSize;
                    uint32_t write_length = block_count * kBlockSize;
                    ret = WriteBuffer(record_file, write_length);
                    assert(ret == 0);
                }
            }

            if (!add_o_frame)
            {
                safe_free(frame->buffer);
                safe_free(frame);
            }

            queue_mutex_.Lock();
        }
        
        if (stop_)
        {
            uint32_t buffer_length = buffer_.length();
            if (buffer_length != 0)
            {
                uint32_t ret;
                RecordFile *record_file = NULL;
                ret = store_client_->GetLastRecordFile(&record_file);
                assert(ret == 0);
                assert(record_file != NULL);

                ret = WriteBuffer(record_file, buffer_length);
                assert(ret == 0);
            }

            break;
        }

        queue_cond_.Wait(queue_mutex_);
    }

    queue_mutex_.Unlock();
    
    Log(logger_, "entry end");
    return;
}

void RecordWriter::Start()
{
    Log(logger_, "Start");

    stop_ = false;
    Create();

    ResetWriteIndexEvent(NULL, WRITE_INDEX_INTERVAL);

    return;
}

void RecordWriter::Stop()
{
    Log(logger_, "stop");

    /* stop queue */
    queue_mutex_.Lock();
    stop_ = true;
    queue_cond_.Signal();
    queue_mutex_.Unlock();
    Join();

    /* clear buffer string */
    String temp;
    temp.swap(buffer_);

    /* clear buffer times*/
    memset(&buffer_times_, 0, sizeof(BufferTimes));

    /* do timer */
    {
        Mutex::Locker lock(store_client_center->timer_lock);
        if (write_index_event_ != NULL)
        {
            store_client_center->timer.DoEvent(write_index_event_);
            write_index_event_ = NULL;
        }
    }

    // finish last record file write
    {
        RecordFile *record_file = NULL;
        ret = store_client_->GetLastRecordFile(&record_file);
        assert(ret == 0 && record_file != NULL);

        record_file->FinishWrite();
    }

    return;
}

// ======================================================= //
//                  store client
// ======================================================= //
StoreClient::StoreClient(Logger *logger, string stream_info)
: logger_(logger), stream_info_(stream_info), record_file_map_(logger_), writer(logger_, this)
{

}

int32_t StoreClient::OpenWrite(uint32_t id)
{
    Log(logger_, "open write, id is %d", id);

    writer.Start();
    
    return 0;
}

int32_t StoreClient::OpenRead(uint32_t id)
{
    Log(logger_, "open read, id is %d", id);
    
    return 0;
}

int32_t StoreClient::EnqueueFrame(FRAME_INFO_T *frame)
{
    assert(frame != NULL);
    assert(frame->buffer != NULL);
    Log(logger_, "enqueue frame %p", frame);

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

    ret = writer->Enqueue(write_op);

    return ret;
}

int32_t StoreClient::GetFreeFile(UTime &time, RecordFile **record_file)
{
    assert(record_file != NULL);
    Log(logger_, "get free file");

    int32_t ret;
    RecordFile *free_file = NULL;

    ret = free_file_table->Get(&free_file);
    assert(ret == 0 && free_file != NULL);

    /* used to record read */
    free_file->stream_info_ = stream_info_;
    free_file->start_time_ = time;
    ret = record_file_map_->PushBackRecordFile(time, free_file);
    assert(ret == 0);

    /* used to recycle */
    ret = store_client_center->AddToRecycleQueue(this, record_file);
    assert(ret == 0);

    *record_file = free_file;
    return 0;
}

int32_t StoreClient::GetLastRecordFile(RecordFile **record_file)
{
    assert(record_file != NULL);
    Log(logger_, "get last record file");

    return record_file_map_->GetLastRecordFile(record_file);
}

int32_t StoreClient::CloseWrite(uint32_t id)
{
    Log(logger_, "close write id %d", id);
    
    writer.Stop();

    return 0;
}

int32_t StoreClient::CloseRead(uint32_t id)
{
    Log(logger_, "close read id %d");

    return 0;
}

// ======================================================= //
//              store client center 
// ======================================================= //
StoreClientCenter::StoreClientCenter(Logger *logger, IdCenter *id_center)
: logger_(logger), rwlock_("StoreClientCenter::RWLock"), recycle_mutex_("StoreClientCenter::RecycleMutex"),
timer_lock("StoreClientCenter::timer_lock"), timer(logger_, timer_lock)
{
    clients_.resize(MAX_STREAM_COUNTS, 0);
}

int32_t OpenStoreClient(int flag, uint32_t id, string &stream_info)
{
    assert(flags == 0 || flags == 1);

    int32_t ret;
    string stream_info;
    StoreClient *client = NULL;

    Log(logger_, "open store client, flag is %d, id is %d, stream info is %s", 
        flag, id, stream_info.c_str());

    if (flags == 0)
    {
        ret = FindStoreClient(stream_info, &client);
        assert(ret == 0 && client != NULL);

        ret = client->OpenRead(id);
    }
    else
    {
        ret = FindStoreClient(stream_info, &client);
        if (ret != 0)
        {
            client = new StoreClient(logger_, stream_info);
            assert(client != NULL);

            RWLock::WRLocker lock(rwlock_);
            clients_[id] = client;
            client_search_map_.insert(make_pair(stream_info, client));
        }

        ret = client->OpenWrite(id);
    }

    Log(logger_, "open store client, flag is %d, id id %d, ret is %d", flag, id, ret);

    return ret;
}

int32_t StoreClientCenter::GetStoreClient(uint32_t id, StoreClient **client)
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

int32_t StoreClientCenter::CloseStoreClient(uint32_t id, int flag)
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

    Log(logger_, "close store client %d, return ret %d", id, ret);

    return ret;
}

int32_t StoreClientCenter::WriteFrame(uint32_t id, FRAME_INFO_T *frame)
{
    assert(frame != NULL);
    Log(logger_, "write frame, id is %d, frame %p", id, frame);

    int32_t ret;
    StoreClient *client = NULL;

    ret = GetStoreClient(id, &client);
    if (ret != 0)
    {
        return ret;
    }

    return client->EnqueueFrame(frame);
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
    Log(logger_, "recycle");

    Mutex::Locker lock(recycle_mutex_);
    dequeue<RecycleItem>::iterator iter = recycle_queue_.begin();
    for (iter; iter != recycle_queue_.end(); iter++)
    {
        RecycleItem recycle_item = *iter;
        RecordFile *record_file = recycle_item.record_file;
        assert(record_file != NULL);
    

    }


}

}
