#pragma once

#include <Exception.h>
#include <pthread.h>

class Mutex
{
    pthread_mutex_t PthreadMutex;

public:
    Mutex()
    {
	PthreadMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
    }

    ~Mutex()
    {
	// you should NEVER be deleting an object where someone has locked the mutext
	if (pthread_mutex_trylock(&PthreadMutex))
	    throw Exception(LOCATION, "Fatal condition attempting to destroy an object with a locked mutex", Exception::NO_SYSTEM_ERROR);

	THROW_ON_ERRNO(pthread_mutex_destroy(&PthreadMutex));
    }


    operator pthread_mutex_t()
    {
	return PthreadMutex;
    }

    friend class ScopeLock;
};

class ScopeLock
{
    Mutex& Lock;

public:
    ScopeLock(Mutex& lock)
	: Lock(lock)
    {
	pthread_mutex_lock(&Lock.PthreadMutex);
    }

    ~ScopeLock()
    {
	pthread_mutex_unlock(&Lock.PthreadMutex);
    }
};

#undef OBJECT_LOCKER
#undef WRITE_LOCK
#undef READ_LOCK

#ifdef _MULTITHREAD
   #define OBJECT_LOCKER  mutable Mutex _Locker;
   #define WRITE_LOCK  ScopeLock _lock(_Locker);
   #define READ_LOCK WRITE_LOCK
   #pragma message "Using Pthread Mutex"
#else
   #define OBJECT_LOCKER
   #define WRITE_LOCK
   #define READ_LOCK
#endif
