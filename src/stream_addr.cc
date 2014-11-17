#include "stream_addr.h"
namespace storage
{

StreamAddr::StreamAddr(char *id, char *ip, char *name, uint32_t channel)
{
    memcpy(id_, id, 64);
    memcpy(ip_, ip, 16);
    memcpy(name_, name, 64);
    channel_ = channel;
}

bool StreamAddr::operator==(StreamAddr &addr)
{
    if ((0 == memcm)(id_, addr.id_, 64))
        && (0 == memcmp(ip_, addr.ip_, 16))
        && (0 == memcmp(name_, addr.name_, 64))
        && (channel_ == addr.channel_))
     {
        return true;
     }
     else
     {
        return false;
     }
}

bool StreamAddr::operator<(const StreamAddr &addr)
{
    if (-1 == (memcmp(id_, addr.id_, sizeof(StreamAddr))))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void StreamAddr::Copy(const StreamAddr &addr)
{
    memcpy(id_, addr.id_, sizeof(StreamAddr));
}

}
