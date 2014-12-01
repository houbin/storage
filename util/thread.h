#ifndef STORAGE_THREAD_H_
#define STORAGE_THREAD_H_

#include <pthread>

namespace storage
{

class Thread
{
private:
    pthread_t thread_id;
    pthread_attr_t *attr;
public:
    Thread() : thread_id(0), attr(NULL) { }

    void Create(size_t stack_size = 0);
    int Join(void **ret = 0);
    int Detach();
    int Kill(int sig);

    virtual void* Entry(void *arg);
};

}

#endif
