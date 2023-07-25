/*
Copyright 2009 by Walt Howard
$Id: Singleton.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <Misc.h>
#include <unistd.h>

template <typename T> class Singleton
{
    static T* item;
    volatile static int counter;
public:

    static T* instance() __attribute__((hot))
    {
	// if instance has been created, get right to returning it.
	while(improbable(not item))
	{
	    // otherwise do allthis rigamarole designed to prevent more than one guy initializing
	    if (__sync_fetch_and_add(&counter, 1) == 0)
	    {
		item = new T;
	    }
	    else
		::sleep(0); // If I'm not the item creator, give up my time slice
	}

	return item;
    }
};

template <typename T> T* Singleton<T>::item = 0;
template <typename T> volatile int Singleton<T>::counter = 0;

template <typename T> class Single: Singleton<T>
{
public:
    T* operator->()  __attribute__((hot))
    {
	return Singleton<T>::instance();
    }
};
