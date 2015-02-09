#ifndef UTIL_RWLOCK_H_
#define UTIL_RWLOCK_H_

#include <pthread.h>
#include <assert.h>

namespace util
{

class RWLock
{
private:
    mutable pthread_rwlock_t rwlock_;
    const char *name_;

public:
    RWLock(const char *name)
    : name_(name)
    {
        pthread_rwlock_init(&rwlock_, NULL);
    }

    ~RWLock()
    {
        pthread_rwlock_destroy(&rwlock_);
    }

    void GetReadLock()
    {
        pthread_rwlock_rdlock(&rwlock_);
    }

    void PutReadLock()
    {
        pthread_rwlock_unlock(&rwlock_);
    }

    void GetWriteLock()
    {
        pthread_rwlock_wrlock(&rwlock_);
    }

    void PutWriteLock()
    {
        pthread_rwlock_unlock(&rwlock_);
    }

    class RDLocker
    {
    private:
        RWLock &rwlock_;

    public:
        RDLocker(RWLock &rwlock)
        : rwlock_(rwlock)
        {
            rwlock_.GetReadLock();
        }

        ~RDLocker()
        {
            rwlock_.PutReadLock();
        }
    };

    class WRLocker
    {
    private:
        RWLock &rwlock_;

    public:
        WRLocker(RWLock &rwlock)
        : rwlock_(rwlock)
        {
            rwlock_.GetWriteLock();
        }

        ~WRLocker()
        {
            rwlock_.PutWriteLock();
        }
    };
};

}

#endif

