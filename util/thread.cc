#include "util/thread.h"
#include <assert.h>

namespace storage
{

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

    ret = pthread_create(&pthread_id, attr, (void*)Entry, NULL);
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
    if (pthread_id == 0)
    {
        assert("join on thread that was never started" == 0);
        return -EINVAL;
    }

    ret = pthread_join(pthread_id, retval);
    assert(ret == 0)
    thread_id = 0;

    return ret;
}

int Thread::Detach()
{
    return pthread_detach(pthread_id);
}

int Thread::Kill(int sig)
{
    if (thread_id)
    {
        ret = pthread_kill(thread_id, sig);
    }
    else
    {
        return -EINVAL;
    }

    return ret;
}

}

