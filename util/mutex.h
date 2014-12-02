#ifndef STORAGE_MUTEX_H_
#define STORAGE_MUTEX_H_
#include <pthread.h>

namespace storage{

class Mutex
{
private:
    const char *name;
    pthread_mutex_t mu_;

    // no copying
    Mutex(const Mutex &);
    void operator=(const Mutex &);

public:
    Mutex();
    ~Mutex();

    void Lock();
    void Unlock();

	friend class Cond;

    class Locker
    {
    private:
        Mutex &mutex_;

    public:
        Locker(Mutex& m) : mutex_(m) { mutex_.Lock(); }
        ~Locker() { mutex_.Unlock(); }
    };
};

}

#endif

