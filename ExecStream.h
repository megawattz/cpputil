/*
Copyright 2009 by Walt Howard
$Id: ExecStream.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <Misc.h>
#include <stdarg.h>
#include <cerrno>
#include <iostream>
#include <vector>
#include <execinfo.h>
#include <fcntl.h>
#include <wait.h>
#include <FileDescriptorStream.h>

/**
 @class  ExecStream
 @brief  Wrap the tedious process of fork/exec/duping pipes etc into an easy package.
 @notes  Enables captures of standard in, out and error (each optionally) and double fork or not.
 */
class ExecStream: public FileDescriptorStream
{
    bool verifyChildRunning() const
    {
        int child_status(0);
        if (_child_pid == ::waitpid(_child_pid, NULL, WNOHANG))
            return false;
        {
            throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Child exited with status: %d - Terminated by: %s", WEXITSTATUS(child_status), WIFEXITED(child_status) ? "exit" : "signal"));
	}
    }

protected:
    Text _command; //> the executable program file (no arguments) that will be the child process.
    typedef std::vector<Text> ArgVector;
    ArgVector _arguments; //> arguments passed to the child process upon startup

    int _error_from_child; //> a file descriptor used to read from the child's standard error
    pid_t _child_pid; //> the child's actual pid

    unsigned _binary_options; //> some options particular to this class (see OPTIONS enum below)

    /**
     @brief Child process itself shouldn't use this contructor. It's for subclasses only.  @note This is necessary due to a
     weakness in c++ constructor initialization lists which cannot pass variable argument lists to parent class
     constructors. Don't try to understand it. You'll understand it when it you try to subclass.
     */
    void parseOptions(const char* commandline, const char* options);

public:
    pid_t GetPid() const
    {
        return _child_pid;
    }

    enum OPTIONS
    {
        NONE = 0x0,
        CAPTURE_STDERR = 0x1,
        CAPTURE_STDOUT = 0x2,
        CAPTURE_STDIN = 0x4,
        CAPTURE_ALL = 0x08,
        DOUBLE_FORK = 0x10
    };

    virtual void open(const char* command = NULL, const char* options = NULL);

    ExecStream(const char* command = NULL, const char* options = "CAPTURE_STDOUT");

    virtual bool eof();

    virtual void close();

    virtual ~ExecStream();
};
