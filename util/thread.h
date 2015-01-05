#ifndef STORAGE_THREAD_H_
#define STORAGE_THREAD_H_

#include <pthread.h>

namespace util
{

class Thread
{
private:
    pthread_t thread_id_;
public:
    Thread();
    virtual ~Thread();

    bool IsStarted() const;
    void Create(size_t stack_size = 0);
    int Join(void **ret = 0);
    int Detach();
    int Kill(int sig);
    virtual void* Entry() = 0;

    static void *EntryWrap(void *arg);
};

}

#endif
