#ifndef STORAGE_m_H_
#define STORAGE_m_H_
#include <pthread.h>

namespace storage{

class Mutex
{
private:
    const char *name;
    pthread_mutex_t m_;

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
        Mutex &m_;

    public:
        Locker(Mutex& m) : m_(m) { m_.Lock(); }
        ~Locker() { m_.Unlock(); }
    };
};

}

#endif

