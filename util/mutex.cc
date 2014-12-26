#include "mutex.h"

namespace util
{

Mutex::Mutex(const char* name)
{
    name_ = name;
    pthread_mutex_init(&m_, NULL);
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&m_);
}

void Mutex::Lock()
{
    pthread_mutex_lock(&m_);
}

void Mutex::Unlock()
{
    pthread_mutex_unlock(&m_);
}


}
