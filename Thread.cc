/*
   Copyright 2009 by Walt Howard
   $Id: Thread.cc 2428 2012-08-14 15:33:13Z whoward $
*/

#include <Thread.h>
#include <signal.h>

boost::thread& Thread::operator->()
{
    return BoostThread;
}


// if used in a boost::thread context, returns the actual boost thread.
boost::thread& Thread::operator&()
{
    return BoostThread;
}


volatile Thread::STATUS Thread::getStatus()
{
    return __sync_add_and_fetch(&Status, 0);
}


void Thread::setStatus(volatile Thread::STATUS status)
{
    __sync_lock_test_and_set(&Status, status);
    __sync_lock_release(&Status);
}


void Thread::ThreadMemberRun()
{
    Status = FINISHED;

    for(;;)
    {
	if (Status >= TERMINATE)
	    break;

	try
	{
	    Hold(); // if we are waiting, an interrupt jogs us loose.
	}
	catch(const boost::thread_interrupted& ex)
	{
	}

	try
	{
	    if (Status >= TERMINATE)
		break;

	    Status = RUNNING;

	    LastException = Exception(LOCATION, "No Error");

	    (ThreadEntry)(ThreadArg);

	    if (Status >= TERMINATE)
		break;
	}
	catch(const IntentionalThreadExit& ex)
	{
	    if (Status >= TERMINATE)
		break;
	    Status = KILLED;
	}
	catch(const boost::thread_interrupted& ex)
	{
	    if (Status >= TERMINATE)
		break;
	    Status = KILLED;
	}
	catch(const Exception& ex)
	{
	    LastException = ex;
	    if (Status >= TERMINATE)
		break;
	    Status = ERROR;
	}
	catch(const std::exception& ex)
	{
	    LastException = Exception(LOCATION, "%s", ex.what());
	    if (Status >= TERMINATE)
		break;
	    Status = ERROR;
	}
	catch(...)
	{
	    LastException = Exception(LOCATION, "Unknown Exception. Caught ...");
	    if (Status >= TERMINATE)
		break;
	    Status = ERROR;
	}
    }
}


void* Thread::ThreadStaticRun(void* arg)
{
    Thread* self = reinterpret_cast<Thread*>(arg);
    self->ThreadMemberRun();
    self->Status = UNTHREADED;
    return 0;
}


void Thread::Stop(const int wait_seconds)
{
    if (Status >= FINISHED)
	return;

    Status = ENDING;

    Interrupt();

    for(int i = 0; i < wait_seconds * 10 and Status < FINISHED; ++i)
	usleep(100000);

    if (Status < FINISHED)
	std::cout << "UNABLE TO STOP THREAD: " << std::endl;
}


void Thread::Release()
{
    if (Status == RUNNING)
	return;

    Interrupt();
}


void Thread::Interrupt()
{
    BoostThread.interrupt();
}


void Thread::Hold(const boost::posix_time::time_duration wait_time)
{
    boost::this_thread::sleep(wait_time);
}


void Thread::Sleep(int msecs)
{
    boost::posix_time::time_duration p = boost::posix_time::milliseconds(msecs);
    boost::this_thread::sleep(p);
}


Thread::Thread(THREADFUNC thread_entry, void* thread_arg)
    : ThreadEntry(0), ThreadArg(0), Status(ENDING), LastException("LOCATION", "Thread::Thread"), BoostThread(ThreadStaticRun, this)
{
    if (thread_entry)
	ThreadEntry = thread_entry;

    if (thread_arg)
	ThreadArg = thread_arg;

    // Block until thread actually starts running.
    while(Status != FINISHED)
	sleep(0);
}


void Thread::Terminate(const int wait_seconds)
{
    // This is tricky. If we've already called terminate, don't interrupt again, however, keep waiting for the
    // the thread to be UNTHREADED.

    int i;

    if (Status >= UNTHREADED)
	return;

    if (Status < TERMINATE)
    {
	Status = TERMINATE;
	BoostThread.interrupt();
    }

    i = 0;
    for(; i < wait_seconds * 10 and Status < UNTHREADED; ++i)
	usleep(100000);

    if (Status >= UNTHREADED)
	return;

    // need to FORCE kill the thread here
    // THIS DOESN"T WORK but is a start at something that might.
    /*
    pthread_t tid = BoostThread.native_handle();
    if (Status < TERMINATE)
    {
	std::cout << "FORCED THREAD KILL. Unusual but not fatal." << tid << std::endl;
	pthread_kill(tid, SIGUSR1);
    }

    i = 0;
    for(; i < wait_seconds * 10 and Status < UNTHREADED; ++i)
	usleep(100000);
    if (Status >= UNTHREADED)
    {
	std::cout << "Thread killed in:" << i << " seconds." << std::endl;
	return;
    }
    */
    std::cout << "UNABLE TO KILL THREAD: " << std::endl;
}


Thread::~Thread()
{
    Terminate(5); // give the thread 5 seconds to terminate gracefully
}


void Thread::Run(THREADFUNC entry = 0, void* thread_arg = 0)
{
    if (Status < FINISHED)
	throw Exception(LOCATION, "Thread is Running Already");

    Status = TRYSTART;

    if (__sync_fetch_and_add(&Status, 1) != TRYSTART)
	throw Exception(LOCATION, "Thread %p Already Started", this);

    if (entry)
	ThreadEntry = entry;

    if (thread_arg)
	ThreadArg = thread_arg;

    Release();
}
