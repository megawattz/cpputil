/*
   Copyright 2009 by Walt Howard
   $Id: Thread.h 2428 2012-08-14 15:33:13Z whoward $
*/
#pragma once

#include <Exception.h>

// workaround for boost error
#undef TIME_UTC
//#define TIME_UTC TIME_UTC_

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>

// Do NOT subclass this from any other exception tree. It's designed to slip through normal
// catches to exit the thread cleanly. It's not an error.
class IntentionalThreadExit : public boost::thread_interrupted {};

typedef void* (*THREADFUNC)(void*);

class Thread
{
    THREADFUNC ThreadEntry;
    void* ThreadArg;

public:
    enum STATUS { TRYSTART,  // Attempt to start
		  STARTED,   // Run called, not threading yet
		  RUNNING,   // is threaded.
		  ENDING,    // is in the process of terminating
		  FINISHED,  // is completely finished running. Can be re-used.
		  KILLED,    // is completely finished running but was killed by an IntentionalThreadExit. Can be re-used.
		  ERROR,     // is completely finished and ended by exception throw indicating an error. Can be re-used.
		  TERMINATE, // Complete death requested.
		  UNTHREADED}; // completely dead, unthreaded, thread terminated, not re-usable

    static const char* StatusToString(int status)
    {
	return SelectString(status, "TRYSTART", "STARTED", "RUNNING", "ENDING", "FINISHED", "KILLED", "ERROR", "TERMINATE", "UNTHREADED");
    }

private:
    volatile STATUS Status;
    Exception LastException;
    static void* ThreadStaticRun(void* arg);

    boost::thread BoostThread; // MUST BE INSTANTIATED LAST!!!!

    void ThreadMemberRun();

    // don't allow copying (copiable Thread is subclass)
    Thread(const Thread& other);
    Thread& operator=(const Thread& other);

public:

    volatile Thread::STATUS getStatus();

    void setStatus(volatile Thread::STATUS status);

    volatile Exception getLastException() { return LastException; }

    Thread(THREADFUNC thread_entry = 0, void* thread_arg = 0);

    // this allows you to call boost thread functions on these objects as if they were boost threads.
    boost::thread& operator->();

    // if used in a boost::thread context, returns the actual boost thread.
    boost::thread& operator&();

    bool Free() const;

    void Terminate(int wait_seconds);

    void Release();

    void Stop(const int wait_seconds);

    void Interrupt();

    void Hold(const boost::posix_time::time_duration wait_time = boost::posix_time::pos_infin);

    void Sleep(int msecs);

    void Run(THREADFUNC entry, void* thread_arg);

    virtual ~Thread();

};
