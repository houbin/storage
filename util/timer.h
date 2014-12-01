#ifndef STORAGE_TIMER_H_
#define STOARGE_TIMER_H_
#include "mutex.h"
#include "logger.h"

#include <map>
using namespace std;

namespace storage
{

class SafeTimerThread : public Thread
{
private:
	SafeTimer *parent_;

public:
	SafeTimerThread(Timer *s) : parent_(s) { }

	void *Entry(void *arg)
	{
		parent_->TimerThread();
		return NULL;
	}
};

class SafeTimer
{
private:
	Logger *logger_;
	Mutex &lock_;
	Cond cond_;
	SafeTimerThread *thread_;

	bool stopping_;

	multimap<UTime, Context*> schedule_;
	map<Context*, multimap<UTime, Context*>::iterator> events_;

public:
	SafeTimer(Logger *logger, Mutex &l) : logger_(logger), lock_(l), thread_(NULL), stopping(false) { }
	~SafeTimer() { assert(thread == NULL); }

	void Init();
	void Shutdown();

	void TimerThread();

	void AddEventAfter(double seconds, Context *callback);
	void AddEventAt(UTime when, Context *callback);

	bool CancelEvent(Context *callback);
	bool CancelAllEvents();
};

}
#endif
