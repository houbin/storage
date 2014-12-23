#ifndef STORAGE_COND_H_
#define STORAGE_COND_H_

#include <pthread.h>
#include <assert.h>
#include "mutex.h"
#include "utime.h"

namespace util
{

class Cond
{
private:
    pthread_cond_t cond_;
    Mutex *waiter_mutex_;    

    // no copying allowed
    Cond(const Cond &);
    void operator=(Cond &c);

public:
    Cond();
    ~Cond();

    int32_t Wait(Mutex &mutex);
    int32_t WaitUtil(Mutex &mutex, UTime time);
    int32_t Signal();
    int32_t SignalAll();
};

}

#endif
