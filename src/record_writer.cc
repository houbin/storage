#include "record_writer.h"
#include "index_file.h"
#include "storage.h"

namespace storage
{

RecordWriter::RecordWriter(Logger *logger,RecordFileMap *file_map)
: logger_(logger), file_map_(file_map), queue_mutex_("RecordWriter::queue_mutex"),
writer_mutex_("RecordWriter::record_file_mutex"), record_file_(NULL), buffer_(NULL), buffer_write_offset_(0),
current_o_frame_(NULL), write_index_event_(NULL), stop_(false)
{
    memset((void *)&buffer_times_, 0, sizeof(BufferTimes));
}

bool RecordWriter::IsStopped()
{
    Mutex::Locker lock(queue_mutex_);
    return stop_;
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
        Mutex::Locker lock(writer_mutex_);

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
    }

    ret = index_file_manager->Find(base_name, &index_file);
    assert(ret == 0 && index_file != NULL);
    file_counts = index_file->GetFileCounts();

    file_info_write_offset = number * sizeof(RecordFileInfo);
    frag_info_write_offset = file_counts * sizeof(RecordFileInfo) + 
                             number * kStripeCount * sizeof(RecordFragmentInfo) + 
                             (record_frag_info_number -1) * sizeof(RecordFragmentInfo);
    index_file->Write(frag_info_write_offset, (char *)&record_frag_info_buffer, sizeof(struct RecordFragmentInfo));
    index_file->Write(file_info_write_offset, (char *)&record_file_info_buffer, sizeof(struct RecordFileInfo));

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
        LOG_INFO(logger_, "reset write index, write_index_event_ is %p", write_index_event_);
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
                
                LOG_INFO(logger_, "O frame, replace current o frame");
                goto FreeResource;
            }
            else if (frame_type == JVN_DATA_I)
            {
                if (current_o_frame_ == NULL)
                {
                    LOG_INFO(logger_, "I frame, but no O frame");
                    goto FreeResource;
                }

                add_o_frame = true;
                first_i_frame = true;
                current_o_frame_->frame_time.seconds = frame->frame_time.seconds;
                current_o_frame_->frame_time.nseconds = frame->frame_time.nseconds;
                LOG_DEBUG(logger_, "I frame, and already have O frame");
            }
            else
            {
                if (first_i_frame == false)
                {
                    LOG_INFO(logger_, "no I frame, frame type is %d, continue", frame_type);
                    goto FreeResource;
                }
            }

            if (add_o_frame)
            {
                frame_length += kHeaderSize + current_o_frame_->size;
            }

            {
                if (record_file_ == NULL)
                {
                    RecordFile *temp_file = NULL;
                    ret = file_map_->OpenWriteRecordFile(&temp_file);
                    if (ret != 0)
                    {
                        ret = file_map_->AllocWriteRecordFile(stamp, &temp_file);
                        assert(ret == 0 && temp_file != NULL);
                    }
                    free_file_table->UpdateDiskWritingStream(temp_file->stream_info_, temp_file);

                    writer_mutex_.Lock();
                    record_file_ = temp_file;
                    writer_mutex_.Unlock();
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
                    RecordFile *record_file = NULL;
                    UTime stamp(frame->frame_time.seconds, frame->frame_time.nseconds);
                    ret = file_map_->AllocWriteRecordFile(stamp, &record_file);
                    assert(ret == 0 && record_file != NULL);

                    writer_mutex_.Lock();
                    record_file_ = record_file;
                    writer_mutex_.Unlock();
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

}

