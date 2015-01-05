#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include "thread.h"

namespace util
{

Thread::Thread()
: thread_id_(0)
{

}

Thread::~Thread()
{

}

bool Thread::IsStarted() const
{
    return (thread_id_ != 0);
}

void Thread::Create(size_t stack_size)
{
    int ret = 0;
    pthread_attr_t *attr = NULL;

    if (stack_size != 0)
    {
        attr = (pthread_attr_t *)malloc(sizeof(pthread_attr_t));
        if (attr == NULL)
        {
            assert("malloc error" == 0);
        }

        pthread_attr_init(attr);
        pthread_attr_setstacksize(attr, stack_size);
    }

    ret = pthread_create(&thread_id_, attr, &EntryWrap, this);
    if (ret != 0)
    {
        assert("pthread_create error" == 0);
    }

    if (attr != NULL)
    { 
        free(attr); 
    }

    return;
}

int Thread::Join(void **retval)
{
    int ret = 0;
    if (thread_id_ == 0)
    {
        assert("join on thread that was never started" == 0);
        return -EINVAL;
    }

    ret = pthread_join(thread_id_, retval);
    assert(ret == 0);
    thread_id_ = 0;

    return ret;
}

int Thread::Detach()
{
    return pthread_detach(thread_id_);
}

int Thread::Kill(int sig)
{
    int ret = 0;

    if (thread_id_)
    {
        ret = pthread_kill(thread_id_, sig);
    }
    else
    {
        return -EINVAL;
    }

    return ret;
}

void* Thread::EntryWrap(void *arg)
{
    void *r = ((Thread *)arg)->Entry();

    return r;
}

}

