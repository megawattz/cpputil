#pragma once

#include<pthread.h>

class Spinlock
{
    volatile int Count;
    volatile pthread_t ThreadId; // for recursive locks
    volatile bool Locked;

public:
    Spinlock();
    ~Spinlock();

    friend class Spinwait;
};

class Spinwait
{
    Spinlock& Lock;

public:
    Spinwait(Spinlock& lock);
    ~Spinwait();
};

#undef OBJECT_LOCKER
#undef WRITE_LOCK
#undef READ_LOCK

#undef CLASS_LOCKER
#undef CLASS_WRITE_LOCK
#undef CLASS_READ_LOCK

#ifdef _MULTITHREAD
   #define OBJECT_LOCKER  mutable Spinlock _Locker;
   #define WRITE_LOCK  Spinwait _lock(_Locker);
   #define READ_LOCK  WRITE_LOCK
   #pragma message "Using Spinlock"
#else
   #define OBJECT_LOCKER
   #define WRITE_LOCK
   #define READ_LOCK
#endif
