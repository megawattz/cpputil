/*
Copyright 2009 by Walt Howard
$Id: ExecStream.cc 2428 2012-08-14 15:33:13Z whoward $
*/

#include <StlHelpers.h>
#include <ExecStream.h>
#include <stdarg.h>
#include <cerrno>
#include <iostream>
#include <vector>
#include <execinfo.h>
#include <fcntl.h>
#include <wait.h>
#include <set>
#include <iostream>
#include <sys/prctl.h>

void ExecStream::parseOptions(const char* commandline, const char* options)
{
    char* token_pos;

    if (commandline)
    {
	char* workstring = strdupa(commandline); // need a mutable copy of the commandline

	const char* command = strtok_r(workstring, " \t\r\n", &token_pos);

	if (not command)
	    _command = "";
	else
	    _command = command;

	_arguments.clear(); // clear out any previous parse

	for (const char* arg = strtok_r(NULL, " \t\r\n", &token_pos); arg; arg
		 = strtok_r(NULL, " \t\r\n", &token_pos)) // extract each workstring argument
	    _arguments.push_back(arg);
    }

    if (not options)
        return;

    if (strstr(options, "CAPTURE_STDOUT"))
        _binary_options |= CAPTURE_STDOUT;

    if (strstr(options, "CAPTURE_STDIN"))
        _binary_options |= CAPTURE_STDIN;

    if (strstr(options, "CAPTURE_STDERR"))
        _binary_options |= CAPTURE_STDERR;

    if (strstr(options, "CAPTURE_ALL"))
        _binary_options |= CAPTURE_ALL;

    if (strstr(options, "DOUBLE_FORK"))
        _binary_options |= DOUBLE_FORK;

    if (_binary_options & DOUBLE_FORK)
    {
	if (_binary_options & CAPTURE_STDOUT)
	    throw Exception(LOCATION, "Cannot capture output of double forked ExecStream");

	if (_binary_options & CAPTURE_STDIN)
	    throw Exception(LOCATION, "Cannot capture input of double forked ExecStream");

	if (_binary_options & CAPTURE_STDERR)
	    throw Exception(LOCATION, "Cannot capture standard error of double forked ExecStream");

	if (_binary_options & CAPTURE_ALL)
	    throw Exception(LOCATION, "Cannot capture handles of double forked ExecStream");
    }
}

void ExecStream::open(const char* commandline, const char* options)
{
    if (commandline)
	set_resource(commandline);

    if (options)
	set_options(options);

    parseOptions(get_resource().c_str(), get_option_string().c_str());

    int child_in[2] = { -2, -2 };
    int child_out[2] = { -2, -2 };
    int child_err[2] = { -2, -2 };

    if (_binary_options & CAPTURE_STDIN)
        THROW_ON_ERROR(::pipe(child_in));

    if (_binary_options & CAPTURE_STDOUT)
        THROW_ON_ERROR(::pipe(child_out));

    if (_binary_options & CAPTURE_STDERR)
        THROW_ON_ERROR(::pipe(child_err));

    pid_t pid = ::fork();
    if (pid < 0)
        THROW_ON_ERROR(pid);

    if (pid != 0)
    {
	_child_pid = pid;
        FileDescriptorStream::set_descriptors(child_out[0], child_in[1]);
	return;
    }

    ::signal(SIGPIPE, SIG_IGN);

    if (_binary_options & CAPTURE_STDIN)
	THROW_ON_ERROR(::dup2(child_in[0], 0));

    if (_binary_options & CAPTURE_STDOUT)
	THROW_ON_ERROR(::dup2(child_out[1], 1));

    if (_binary_options & CAPTURE_STDERR)
	THROW_ON_ERROR(::dup2(child_err[1], 2));

    /*
      DOUBLE_FORK lets us run the child completely independently. It will be owned by init.
    */

    if (_binary_options & DOUBLE_FORK)
    {
	prctl(PR_SET_PDEATHSIG, 0);

	pid_t pid2 = ::fork();

	if (pid2 < 0)
	    throw Exception(LOCATION, "fork failed: ");

	//You are now doubly forked.

	//Exiting here satisfies our parent's waitpid, while our child continues on running
	//owned by the linux root process, init.

	if (pid2 > 0)
	    exit(EXIT_SUCCESS);
    }

    std::vector<char*> argument_cstrs;

    argument_cstrs.push_back(const_cast<char*> (_command.c_str())); // the exec file is also argv[0]

    for (ArgVector::const_iterator arg(_arguments.begin()); arg != _arguments.end(); ++arg)
	argument_cstrs.push_back(strdup(arg->c_str()));

    //std::cerr << _command << ' ' << Join(argument_cstrs, ' ') << std::endl;

    argument_cstrs.push_back(0);

    if (::execvp(_command.c_str(), &argument_cstrs[0]) < 0)
	std::cerr << _command << " failed. " << std::strerror(errno)
		  << std::endl;

    /*
       This will never execute on a successful exec. You must terminate here. We are in the child process now and throwing an exception or
       returning an error or doing anything except dying will result in an additional copy of your parent program running which is probably the
       last thing you want.
    */
    std::cout << "execvp in child failed (parent still running): " << _command << ':' << std::strerror(errno) << std::endl;
    exit(errno);
}

ExecStream::ExecStream(const char* command, const char* options) :
    FileDescriptorStream(command, options), _error_from_child(-2) , _child_pid(-1), _binary_options(0)
{
    parseOptions(command, options);
}

void ExecStream::close()
{
    // ignore errors. If a handle is invalid here it's not going to matter as we are destroying the existence
    // of this object.
    FileDescriptorStream::close();

    // If we didn't double fork, we are responsible for cleaning up the zombie. Do this just in case we
    // didn't do it elsewhere. Ignore the error as it might have already been done. We are just making sure.

    // if (not (_binary_options & DOUBLE_FORK))
    {
        if (_child_pid > 0)
            ::kill(_child_pid, SIGKILL);

        //::waitpid(_child_pid, NULL, WNOHANG);
        ::waitpid(_child_pid, NULL, 0);
    }

}

bool ExecStream::eof()
{
    fillBuffer(); // make sure to read from the child output, otherwise its output may block and it will never die.
    bool nobuffered = not hasBuffered();
    bool dead = (_child_pid == ::waitpid(_child_pid, NULL, WNOHANG));
    return nobuffered and dead;
}

ExecStream::~ExecStream()
{
    close();
}
