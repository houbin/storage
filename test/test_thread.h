#include <stdio.h>
#include "../util/cond.h"
#include "../util/mutex.h"
#include "../util/thread.h"

using namespace util;

class Receiver : public Thread
{
private:
    int id_;
public:
    Receiver(int id) : id_(id)
    {}

    void *Entry();
};

class Signaler: public Thread
{
private:
    int id_;
public:
    Signaler(int id) : id_(id)
    {}

    void *Entry();
};

