#ifndef STORAGE_STREAM_ADDR_H_
#define STORAGE_STREAM_ADDR_H_   

namespace storage
{

class StreamAddr
{
private:
    char id_[64];        //云视通id
    char ip_[16];        
    char name_[64];
    uint32_t channel_;

    StreamAddr(const Streamaddr& );
    void operator=(const StreamAddr& );
public:
    StreamAddr(char *id, char *ip, char *name, uint32_t channel);
    ~StreamAddr() {}

    bool operator==(StreamAddr &addr);
    bool operator<(const StreamAddr &addr);
    void Copy(const StreamAddr &addr);
};


}
#endif
