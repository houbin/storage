#include "test_thread.h"
#include <unistd.h>

using namespace util;

Cond cond;
Mutex mutex("mutex");

void *Receiver::Entry()
{
    mutex.Lock();
    cond.Wait(mutex);
    mutex.Unlock();

    fprintf(stderr, "id is %d\n", id_);

    return 0;
}


void *Signaler::Entry()
{
    mutex.Lock();
    cond.SignalAll();
    mutex.Unlock();
    return 0;
}

int main()
{
    int count = 2;
    Receiver **array = new Receiver*[count];

    Signaler *s = new Signaler(5);
    s->Create();

    for (int i = 0; i < count; i++)
    {
        array[i] = new Receiver(i);
        array[i]->Create();
    }


    for (int i = 0; i < count; i++)
    {
        array[i]->Join();
        delete array[i];
    }

    s->Join();
    delete s;
    delete []array;

    return 0;
}
