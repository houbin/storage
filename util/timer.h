#ifndef STORAGE_TIMER_H_
#define STOARGE_TIMER_H_
#include "mutex.h"
#include "logger.h"

#include <map>
using namespace std;

namespace storage
{
class Timer
{
private:
	Mutex &mutex;
	Logger *info_log;
	Cond cond;
	TimeThread *timer_thread;

	multimap<struct timeval, Context*> schedule;
	map<Context*, multimap<struct timeval, Context*>::iterator> events;
	bool stopping;

public:
	
	



}


}

#endif
