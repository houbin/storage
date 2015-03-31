#ifndef UTIL_ATOMIC_H_
#define UTIL_ATOMIC_H_

namespace util
{

template<typename T>
class AtomicIntegerT
{
private:
    volatile T value_;

public:
    AtomicIntegerT()
    : value_(0)
    {
    }

    void Inc()
    {
        __sync_fetch_and_add(&value_, 1);
        return;
    }

    void Dec()
    {
        __sync_fetch_and_add(&value_, -1);
        return;
    }

    T Get()
    {
       return  __sync_val_compare_and_swap(&value_, 0, 0);
    }
};

typedef AtomicIntegerT<int32_t> AtomicInt32;
typedef AtomicIntegerT<int64_t> AtomicInt64;

}

#endif

