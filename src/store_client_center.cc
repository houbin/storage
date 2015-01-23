#include "store_client_center.h"
#include "../util/coding.h"

namespace storage
{
// ======================================================= //
//                  record file map
// ======================================================= //
RecordFileMap::RecordFileMap(Logger *logger)
: logger_(logger), rwlock_("RecordFileMap::rwlock"), stop_(false)
{

}

/* 找到time所在的record file, 返回-1表示没有找到*/
int32_t RecordFileMap::GetRecordFile(UTime time, RecordFile **record_file)
{
    Log(logger_, "get record file time %d.%d", time.tv_sec, time.tv_nsec);
    map<UTime, RecordFile*>::iterator iter_up;
    map<UTime, RecordFile*>::iterator iter;

    RWLock::RDLocker locker(rwlock_);
    if (stop_)
    {
        return -1;
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
    Log(logger_, "get last record file");
    map<UTime, RecordFile*>::iterator iter;

    RWLock::RDLocker locker(rwlock_);
    if (stop_)
    {
        return -1;
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
    pair<map<UTime, RecordFile*>::iterator, bool> ret;

    assert(record_file != NULL);
    Log(logger_, "push back record file, time %d.%d", time.tv_sec, time.tv_nsec);

    RWLock::WRLocker locker(rwlock_);
    if (stop_)
    {
        return -1;
    }

    ret = record_file_map_.insert(make_pair(time, record_file));
    if (ret->second == false)
    {
        assert("record file already exist" == 0);
        return -1;
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
StoreClient::Writer::Writer(Logger *logger, StoreClient *store_client)
: logger_(logger), store_client_(store_client), queue_mutex_("Writer::locker"), current_o_frame(NULL), stop_(false)
{
    memset(buffer_times_, 0, sizeof(BufferTimes));
}

int32_t StoreClient::Writer::EncodeHeader(char *header, FRAME_INFO_T *frame)
{
    Log(logger_, "encode header");

    assert(header != NULL);
    assert(frame != NULL);

    EncodeFixed32(header, kMagicCode);
    EncodeFixed32(header, frame->type);
    EncodeFixed32(header, frame->frame_time.seconds);
    EncodeFixed32(header, frame->frame_time.nseconds);
    EncodeFixed64(header, frame->stamp);
    EncodeFixed32(header, frame->size);

    return 0;
}

int32_t StoreClient::Writer::EncodeFrame(bool add_o_frame, FRAME_INFO_T *frame)
{
    assert(frame != NULL);
    Log(logger_, "encode frame, add_o_frame is %d", add_o_frame);

    if (add_o_frame)
    {
        char header[kHeaderSize] = {0}; 
        EncodeHeader(header, current_o_frame); 
        buffer_.append(header, kHeaderSize); 
        buffer_.append(current_o_frame->buffer, current_o_frame->size);
    }

    char header[kHeaderSize] = {0}; 
    EncodeHeader(header, frame); 
    buffer_.append(header, kHeaderSize); 
    buffer_.append(frame->buffer, frame->size); 

    return 0;
}


int32_t StoreClient::Writer::Enqueue(WriteOp *write_op)
{
    Log(logger_, "writer enqueue write_op %p", write_op);

    if (stop_)
    {
        Log(logger_, "have stopped, so should't go here");
        return -1;
    }

    Mutex::Locker lock(queue_mutex_);
    write_op_queue_.push_back(write_op);
    queue_cond_.Signal();

    return 0;
}

int32_t StoreClient::Writer::Dequeue(WriteOp **write_op)
{
    Log(logger_, "writer enqueue write_op %p", write_op);
    *write_op = write_op_queue_.front();
    write_op_queue_.pop_front();

    return 0;
}

int32_t StoreClient::Writer::UpdateBufferTimes(uint32_t type, UTime time);
{
    Log(logger_, "update buffer times");

    if (buffer_times_.start_time == 0)
    {
        buffer_times_.start_time = time;
    }

    if ((buffer_times_.i_frame_start_time == 0)
        && (type == JVN_DATA_I))
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

int32_t StoreClient::Writer::WriteBuffer(uint32_t write_length, RecordFile *record_file)
{
    assert(record_file != NULL);
    Log(logger_, "write buffer, write_length is %d, record file base name is %s, number is %d", 
            write_length, record_file->base_name_, record_file->number_);

    ret = reocrd_file->Append(buffer_, write_length, buffer_times_);
    assert(ret == 0);

    buffer_.erase(0, write_length);
    memset(&buffer_times_, 0, sizeof(BufferTimes));

    return 0;
}

int32_t StoreClient::Writer::Entry()
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
            uint32_t frame_type = frame->type;
            bool add_o_frame = false;
            if (frame_type == JVN_DATA_O)
            {
                if (current_o_frame != NULL)
                {
                    safe_free(current_o_frame->buffer);
                    safe_free(current_o_frame);
                }

                current_o_frame = frame;
            }
            else
            {
                if (current_o_frame == NULL)
                {
                    safe_free(frame->buffer);
                    safe_free(frame);
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
                frame_length += kHeaderSize + current_o_frame->size;
            }

            /* the very first frame: must be o frame */
            RecordFile *record_file = NULL;
            ret = store_client_->record_file_map_->GetLastRecordFile(&record_file);
            if (ret == ERR_ITEM_NOT_FOUND)
            {
                Log(logger_, "doesn't find any record file");
                ret = store_client_->GetFreeFile(frame->frame_time, &record_file);
                assert(ret == 0);
                assert(record_file != NULL);
            }

            /* 获取record file的写偏移量 */
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
                    ret = WriteBuffer(write_length, record_file);
                    assert(ret == 0);
                }
            }
            else
            {
                /* no enough space in record file */
                if (buffer_old_length != 0)
                {
                    uint32_t write_length = buffer_old_length;
                    ret = WriteBuffer(write_length, record_file);
                    assert(ret == 0);
                }

                /* append数据 */
                EncodeFrame(add_o_frame, frame);

                /* 更新当前buffer的时间 */
                UpdateBufferTimes(type, frame->frame_time);

                RecordFile *record_file = NULL;
                ret = store_client_->GetFreeFile(frame->frame_time, &record_file);
                assert(ret == 0);
                assert(record_file != NULL);

                uint32_t buffer_length = buffer_.length();
                if (buffer_length >= kBlockSize)
                {
                    uint32_t block_count = buffer_length / kBlockSize;
                    uint32_t write_length = block_count * kBlockSize;
                    ret = WriteBuffer(write_length, record_file);
                    assert(ret == 0);
                }
            }

            queue_mutex_.Lock();
        }
        
        if (stop_)
        {
            uint32_t ret;
            RecordFile *record_file = NULL;
            ret = store_client_->GetLastRecordFile(&record_file);
            assert(ret == 0);
            assert(record_file != NULL);

            uint32_t buffer_length = buffer_.length();
            if (buffer_length != 0)
            {
                ret = WriteBuffer(buffer_length, record_file);
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

void StoreClient::Writer::Stop()
{
    Log(logger_, "stop");

    queue_mutex_.Lock();
    stop_ = true;
    queue_cond_.Signal();
    queue_mutex_.Unlock();

    Join();

    return 0;
}


// ======================================================= //
//                  store client
// ======================================================= //
StoreClient::StoreClient(Logger *logger, string stream_info)
: logger_(logger), stream_info_(stream_info), rwlock_("StoreClient::RWLock")
{
    Log("storge client construct");

    writer.Create();
}

int32_t StoreClient::Open(int flags, uint32_t id);
{
    if (flags == 0)
    {
        /* TODO: do something */
    }
    
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

    writer->Enqueue(write_op);

    return 0;
}

int32_t StoreClient::GetFreeFile(UTime &time, RecordFile **record_file)
{
    assert(record_file != NULL);

    int32_t ret;
    RecordFile *free_file = NULL;
    Log(logger_, "get free file");

    ret = free_file_table_->Get(&free_file);
    assert(ret == 0);
    assert(free_file != NULL);
    free_file->stream_info_ = stream_info_;
    free_file->start_time_ = time;

    ret = record_file_map_->PushBackRecordFile(time, free_file);
    assert(ret == 0);

    *record_file = free_file;
    return 0;
}

int32_t StoreClient::GetRecordFile(UTime time, RecordFile **record_file)
{

}

int32_t StoreClient::GetLastRecordFile(RecordFile **record_file)
{

}

int32_t StoreClient::PushBackRecordFile(RecordFile *record_file)
{

}

int32_t Stop(int fd)
{
    
}

// ======================================================= //
//              store client center 
// ======================================================= //
StoreClientCenter::StoreClientCenter(Logger *logger)
: logger_(logger), mutex_(Center::Locker)
{

}

int32_t OpenStoreClient(int flags, uint32_t id, string &stream_info)
{
    int32_t ret;
    string stream_info;
    StoreClient *client = NULL;

    assert(flags == 0 || flags == 1);

    Log(logger_, "add store client, id is %d", id);

    Mutex::Locker lock(mutex_);
    
    map<string, StoreClient*>::iterator iter = client_search_map_.find(stream_info);
    if (iter == client_search_map_.end())
    {
        if (flags == 0)
        {
            return -ERR_STREAM_NOT_FOUND;
        }

        client = new StoreClient(logger_, stream_info);
        assert(client != NULL);

        clients_[id] = client;
        client_search_map_.insert(make_pair(stream_info, client));
    }
    else
    {
        client = iter->second;
    }

    ret = client->Open(flags, id);
    if (ret != 0)
    {
        Log(logger_, "open flags %d id %d error", flags, id);
        return ret;
    }

    return 0;
}


}
