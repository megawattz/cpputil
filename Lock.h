#pragma once

#include <boost/thread/recursive_mutex.hpp>

/*
   Adds a mutex to a class that otherwise doesn't have one so the class can be used by Locked
*/
template <typename LOCKEE> class Lockable: public LOCKEE
{
    mutable boost::recursive_mutex Locker;

public:
    void Lock() const __attribute__((hot))
    {
	Locker.lock();
    }

    void Unlock() const  __attribute__((hot))
    {
	Locker.unlock();
    }
};


template <typename LOCKABLE> class Locked
{
    LOCKABLE* Instance;

    // Only allow temporaries (prevent holding locks longer than one call)
    Locked<LOCKABLE>(const Locked<LOCKABLE>&);
    Locked<LOCKABLE> operator=(const Locked<LOCKABLE>&);
    static void* operator new(size_t);

    Locked(LOCKABLE* lockee)
	: Instance(lockee)
    {
	Instance->Lock();
    }

public:
    ~Locked()  __attribute__((hot))
    {
	Instance->Unlock();
    }


    LOCKABLE* operator->()  __attribute__((hot))
    {
	return Instance;
    }


    const LOCKABLE* operator->() const  __attribute__((hot))
    {
	return Instance;
    }

    LOCKABLE& operator*()  __attribute__((hot))
    {
	return *Instance;
    }

    template <typename LOCKEE> friend Locked<LOCKEE> Lock(LOCKEE* lockee);
};


template <typename LOCKEE> Locked<LOCKEE> Lock(LOCKEE* lockee)
{
    return lockee;
}
