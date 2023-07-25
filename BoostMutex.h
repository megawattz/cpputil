#pragma once

#include <boost/thread/recursive_mutex.hpp>

class BoostMutex: public boost::recursive_mutex
{
public:
    friend class BoostWriteLock;
};

class BoostWriteLock
{
    BoostMutex& Locker;

public:
    BoostWriteLock(BoostMutex& locker)
	: Locker(locker)
    {
	Locker.lock();
    }

    ~BoostWriteLock()
    {
	Locker.unlock();
    }
};

#undef OBJECT_LOCKER
#undef WRITE_LOCK
#undef READ_LOCK

#ifdef _MULTITHREAD
   #define OBJECT_LOCKER  mutable BoostMutex _Locker;
   #define WRITE_LOCK  BoostWriteLock _lock(_Locker);
   #define READ_LOCK  WRITE_LOCK
   #pragma message "Using Boost Mutex"
#else
   #define OBJECT_LOCKER
   #define WRITE_LOCK
   #define READ_LOCK
#endif
