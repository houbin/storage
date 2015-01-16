#include "stream_transfer_client.h"
#include "../include/JVNSDKDef.h"
#include "../include/JVNSDK.h"
#include "../include/JvClient.h"
#include "yst_userdef.h"
#include "stream_info.h"

const static int kWriteCacheSize = 1024 * 1024;
namespace storage
{

StreamTransferClient::StreamTransferClient(Logger *logger)
: logger_(logger), mutex_(StreamTransferClient::Locker), stop_(false), write_offset_(0)
{
    write_buffer_= (unsigned char *)malloc(kWriteCacheSize);
    assert(write_buffer_ != NULL);
}

int32_t GetStreamInfo(StreamInfo **stream_info)
{
    Log(logger_, "get stream info");
    assert(stream_info != NULL);

    *stream_info = &(this->stream_info_);

    return 0;
}

int32_t StreamTransferClient::Start()
{
    Log(logger_, "start");

    Mutex::Locker lock(mutex_);
    stop_ = false;

    JVC_Connect(0, stream_id, stream_info_.stream_server_channel_id_, stream_info_.stream_server_ip_,
        stream_info_.stream_server_port_, stream_info_.user_name_, stream_info_.user_passwd_,
        -1, "", true, 1, false, NULL, false);

    return 0;
}

bool StreamTransferClient::IsStop()
{
    Log(logger_, "judge is stop?");
    Mutex::Locker lock(mutex_);

    return stop_;
}

int32_t StreamTransferClient::Stop()
{
    Log(logger_, "stop, stream id is %d", stream_info_.stream_id_);

    Mutex::Locker lock(mutex_);
    stop_ = true;

    JVC_DisConnect(stream_info_.stream_id_);

    return 0;
}

bool StreamTransferClient::ShouldShutdown()
{
    Log(logger_, "judge should shutdown?");

    Mutex::Locker lock(mutex_);
    if (stop_ && record_file_map_.empty())
    {
        return true;
    }

    return false;
}

int32_t StreamTransferClient::Shutdown()
{
    Log(logger_, "shutdown");

    if (write_buffer_ != NULL)
    {
        free(write_buffer_);
    }

    return 0;
}

/* 系统初始化时，通过解析index文件，将已经录像的文件插入到对应流的client中 */
int32_t Insert(RecordFile *record_file)
{
    Log(logger_, "insert record file");

    assert(record_file != NULL);
    Mutex::Locker lock(mutex_);
    record_file_map_.insert(make_pair(record_file->start_time_, record_file));

    return 0;
}

int32_t RecycleExpiredRecordFiles(list<RecordFile*> &expired_file_list, UTime expired_deadline)
{
    Log(logger_, "recycle expired record file");

    Mutex::Locker lock(mutex_);
    map<UTime, RecordFile*>::iterator iter = record_file_map_.begin();
    while (iter != record_file_map_.end())
    {
        RecordFile *record_file = iter->second;
        if (record_file->end_time_ >= expired_deadline)
        {
            break;
        }

        if (record_file->locked_)
        {
            Log(logger_, "record file base name %s, number %d locked, continue", 
                        record_file->base_name_, record_file->number_);
            iter++;
            continue;
        }

        map<UTime, RecordFile*>::iterator to_erase = iter;
        iter++;
        record_file_map_.erase(to_erase);

        record_file->Clear();
        expired_file_list.push_back(record_file);
    }

    Log(logger_, "recycle expired record file end");

    return 0;
}

int32_t Store(unsigned char type, unsigned char *buffer, int size, int width, int height)
{
    Log(logger_, "store, type %d, buffer %p, size %d, width %d, height %d", 
        type, buffer, width, height);


}

}

