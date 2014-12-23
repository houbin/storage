#include <stdio.h>
#include "timer.h"
#include "context.h"
#include "logger.h"

namespace storage
{

class OSD;
class ContextA : public Context
{
private:
    Logger *logger_;
    OSD *osd_;
public:
    friend class OSD;
    ContextA(Logger *logger, OSD *osd) : logger_(logger), osd_(osd){}

    void Finish(int r);
};

class OSD
{
private:
    Logger *logger_;
    SafeTimer *timer_;

public:
    OSD(Logger *logger, SafeTimer *timer) : logger_(logger), timer_(timer) {}
    void Tick();
};

void ContextA::Finish(int r)
{
    Log(logger_, "ContextA finish");
    osd_->Tick();

    return;
}

void OSD::Tick()
{
    ContextA *a = new ContextA(logger_, this);
    timer_->AddEventAfter(2.0, a);

    return;
}

}

using namespace storage;

int main()
{
    int32_t ret;
    Logger *logger = NULL;

    ret = NewLogger("/tmp/timer_test.log", &logger);
    if (ret != 0)
    {
        printf("Newlogger error, ret is %d\n", ret);
        return ret;
    }

    Mutex lock;
    SafeTimer *timer = new SafeTimer(logger, lock);
    timer->Init();

    OSD *osd = new OSD(logger, timer);
    osd->Tick();

    sleep(20);
    {
        Mutex::Locker l(lock);
        timer->Shutdown();
    }

    if (logger != NULL)
    {
        delete logger;
    }

    if (timer != NULL)
    {
        delete timer;
    }

    return 0;
}
