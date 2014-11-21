#include "stream_map.h"
#include "errcode.h"
namespace storage
{

int32_t StreamMap::Open(StreamAddr &addr, bool create_if_missing, Stream **stream)
{
    bool found;

    Mutex::Locker locker(mutex_);
    map<StreamAddr, Stream>::iterator iter;
    iter = stream_map_.find(addr);
    if (iter == stream_map.end())
    {
        found = false;
    }
    else
    {
        found = true;
    }

    if (found)
    {
        *stream = &(iter->second);
    }
    else if (create_if_missing)
    {
        Stream stream;

        stream.CopyAddr(addr);
        s.write_buffer_ = (char *)malloc(sizeof(BUFFER_LENGTH));
        s.read_buffer = (char *)malloc(sizeof(BUFFER_LENGTH));
        if ((NULL == s.write_buffer_)
            || (NULL == s.read_buffer_))
        {
            return ERR_MALLOC_FAILED;
        }

        pair<map<StreamAddr, Stream>::iterator, bool> ret;
        ret = stream_map_.insert(make_pair(addr, stream));
        if (false == ret->second)
        {
            return ERR_ENTRY_INSERT_FAILED;
        }

        *stream = &(ret->first->second);
    }
    else
    {
        return ERR_ENTRY_NOT_FOUND;
    }

    return OK;
}

int32_t StreamMap::Close(Stream *stream)
{
    return OK;
}

int32_t StreamMap::Delete(Stream *stream)
{
    if (NULL == stream)
    {
        return ERR_INVALID_INPUT_PARA;
    }
    
    if (NULL != stream->write_buffer_)
    {
        free(stream->write_buffer_);
        stream->write_buffer_ = NULL;
    }

    if (NULL != stream->read_buffer_)
    {
        free(stream->read_buffer_);
        stream->read_buffer_ = NULL;
    }

    Mutex::Locker locker(mutex_);
    stream_map_.erase(stream->addr_);

    return OK;
}


}
