#ifndef STORAGE_TIMER_H_
#define STOARGE_TIMER_H_
#include "thread.h"
#include "mutex.h"
#include "logger.h"
#include "cond.h"
#include "utime.h"
#include "context.h"

#include <map>
using namespace std;

namespace util
{

class SafeTimerThread;

class SafeTimer
{
private:
    Logger *logger_;
    Mutex &mutex_;
    Cond cond_;
    SafeTimerThread *thread_;

    bool stop_;

    multimap<UTime, Context*> schedule_;
    map<Context*, multimap<UTime, Context*>::iterator> events_;

public:
    SafeTimer(Logger *logger, Mutex &mutex) : logger_(logger), mutex_(mutex), thread_(NULL), stop_(false) { }
    ~SafeTimer() { assert(thread_ == NULL); }

    void Init();
    void Shutdown();

    void TimerThread();

    void AddEventAfter(double seconds, Context *callback);
    void AddEventAt(UTime when, Context *callback);

    void DoEvent(Context *callback);
    void DoAllEvents();
    bool CancelEvent(Context *callback);
    bool CancelAllEvents();
};

class SafeTimerThread : public Thread
{
private:
    SafeTimer *parent_;

public:
    SafeTimerThread(SafeTimer *s) : parent_(s) { }

    void *Entry()
    {
        parent_->TimerThread();
        return NULL;
    }
};

}
#endif

