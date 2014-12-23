#include "clock.h"
#include "logger.h"
#include "timer.h"
#include "context.h"
#include "utime.h"

using namespace std;

namespace util
{

void SafeTimer::Init()
{
    Log(logger_, "init");
    thread_ = new SafeTimerThread(this);
    thread_->Create();
}

void SafeTimer::Shutdown()
{
    Log(logger_, "shutdown");
    if (thread_ != NULL)
    {
        CancelAllEvents();
        stopping_ = true;
        cond_.Signal();
        lock_.Unlock();
        thread_->Join();
        lock_.Lock();
        delete thread_;
        thread_ = NULL;
    }
}

typedef std::multimap<UTime, Context*> scheduled_map_t;
typedef std::map<Context*, scheduled_map_t::iterator> event_lookup_map_t;

void SafeTimer::TimerThread()
{
    Log(logger_, "TimerThread starting");

    lock_.Lock();
    while (!stopping_)
    {
        UTime t = GetClockNow();

        while (!schedule_.empty())
        {
            scheduled_map_t::iterator iter = schedule_.begin();

            if (iter->first > t)
                break;

            Context *callback = iter->second;

            events_.erase(callback);
            schedule_.erase(iter);

            Log(logger_, "TimerThread executing %p", callback);
            callback->Complete(0);
        }

        Log(logger_, "TimerThread going to sleep");

        if(schedule_.empty())
        {
            cond_.Wait(lock_);
        }
        else
        {
            cond_.WaitUtil(lock_, schedule_.begin()->first);
        }

        Log(logger_, "TimerThread awake");
    }

    Log(logger_, "TimerThread exiting");
    lock_.Unlock();
}

void SafeTimer::AddEventAfter(double seconds, Context *callback)
{
    UTime t = GetClockNow();

    t += seconds;

    AddEventAt(t, callback);

    return;
}

void SafeTimer::AddEventAt(UTime t, Context* callback)
{
    Log(logger_, "AddEventAfter %d.%d -> %p", t.tv_sec, t.tv_nsec, callback);

    scheduled_map_t::value_type s_val(t, callback);
    scheduled_map_t::iterator i = schedule_.insert(s_val);

    event_lookup_map_t::value_type e_val(callback, i);
    pair<event_lookup_map_t::iterator, bool> rval(events_.insert(e_val));

    assert(rval.second);

    if (i == schedule_.begin())
    {
        cond_.Signal();
    }

    return;
}

bool SafeTimer::CancelEvent(Context *callback)
{
    map<Context*, multimap<UTime, Context *>::iterator>::iterator p = events_.find(callback);
    if (p == events_.end())
    {
        Log(logger_, "CancelEvents %p not found", callback);
        return false;
    }

    Log(logger_, "CancelEvent %d.%d -> %p", p->second->first.tv_sec, p->second->first.tv_nsec, callback);
    Context *ct = p->first;

    events_.erase(p);
    schedule_.erase(p->second);

    if(ct != NULL)
    {
        delete ct;
    }

    return true;
}

bool SafeTimer::CancelAllEvents()
{
    while(!events_.empty())
    {
        map<Context*, multimap<UTime, Context*>::iterator>::iterator p = events_.begin();
        Log(logger_, "cancelled %d.%d -> %p", p->second->first.tv_sec, p->second->first.tv_nsec, p->first);

        Context *ct = p->first;
        events_.erase(p);
        schedule_.erase(p->second);
        if (ct != NULL)
        {
            delete ct;
        }
    }

    return true;
}

}
