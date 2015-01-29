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
        /* 先关掉定时器处理线程 */
        mutex_.Lock();
        stop_ = true;
        cond_.Signal();
        mutex_.Unlock();
        thread_->Join();
        delete thread_;
        thread_ = NULL;

        /* 然后处理定时器队列中的event */
        mutex_.Lock();
        DoAllEvents();
        mutex_.Unlock();
    }

    return;
}

typedef std::multimap<UTime, Context*> scheduled_map_t;
typedef std::map<Context*, scheduled_map_t::iterator> event_lookup_map_t;

void SafeTimer::TimerThread()
{
    Log(logger_, "TimerThread starting");

    mutex_.Lock();
    while (!stop_)
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

            //mutex_.Unlock();

            Log(logger_, "TimerThread executing %p", callback);
            callback->Complete(0);

            //mutex_.Lock();
        }

        if (stop_)
        {
            break;
        }

        Log(logger_, "TimerThread going to sleep");

        if(schedule_.empty())
        {
            cond_.Wait(mutex_);
        }
        else
        {
            cond_.WaitUtil(mutex_, schedule_.begin()->first);
        }

        Log(logger_, "TimerThread awake");
    }

    Log(logger_, "TimerThread exiting");
    mutex_.Unlock();

    return;
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

    if (stop_)
    {
        delete callback;
        return;
    }

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

void SafeTimer::DoEvent(Context *callback)
{
    Log(logger_, "do event, callback is %p", callback);

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
    
    ct->Complete(-1);

    return;
}

void SafeTimer::DoAllEvents()
{
    Log(logger_, "do all events");

    while (!schedule_.empty())
    {
        scheduled_map_t::iterator iter = schedule_.begin();
        Context *ct = iter->second;
        assert(ct != NULL);
        events_.erase(ct);
        schedule_.erase(iter);

        mutex_.Unlock();
        ct->Complete(0);
        mutex_.Lock();
    }

    return;
}

bool SafeTimer::CancelEvent(Context *callback)
{
    Log(logger_, "cancle event %p", callback);

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
    Log(logger_, "cancle all events");

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
