#include <pthread.h>
#include <assert.h>
#include "mutex.h"
#include "utime.h"
#include "cond.h"

namespace util
{

Cond::Cond()
: waiter_mutex_(NULL)
{
    int r = pthread_cond_init(&cond_, NULL);
    assert(r == 0);
}

Cond::~Cond()
{
    int r = pthread_cond_destroy(&cond_);
    assert(r == 0);
}

int32_t Cond::Wait(Mutex &mutex)
{
    assert(waiter_mutex_ == NULL || waiter_mutex_ == &mutex);
    waiter_mutex_ = &mutex;

    int r = pthread_cond_wait(&cond_, &mutex.m_);
    return (int32_t)r;
}

int32_t Cond::WaitUtil(Mutex &mutex, UTime time)
{
    assert(waiter_mutex_ == NULL || waiter_mutex_ == &mutex);

    waiter_mutex_ = &mutex;

    struct timespec ts;
    time.ToTimeSpec(&ts);

    int r = pthread_cond_timedwait(&cond_, &mutex.m_, &ts);

    return (int32_t)r;
}

int32_t Cond::Signal()
{
    int r = pthread_cond_signal(&cond_);

    return (int32_t)r;
}

int32_t Cond::SignalAll()
{
    int r = pthread_cond_broadcast(&cond_);
    
    return r;
}

}
