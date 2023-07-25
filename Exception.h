/*
Copyright 2009 by Walt Howard
$Id: Exception.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <Misc.h>
#include <StackTrace.h>
#include <exception>
#include <cerrno>
#include <string>
#include <cstring>


/**
 These macros help to quickly turn old style c functions into c++ functions, i.e., they will make the wrapped call throw an exception upon error
 instead of only returning an error code.

 Examples: THROW_ON_ERROR(printf("This better work"));
 THROW_ON_FALSE(fopen("afilename.txt", "r"));

 If this process of checking errors isn't made easy, programmers neglect it.
 */

// all the wierd and quirky ways non exception code returns errors. You can use these macros to wrap your function call
// and it will throw an exception if the function returns an error condition.

// typical Unix/Linux errors
#define THROW_ON_ERROR(FUNCTION_CALL) { if (improbable(-1 == (FUNCTION_CALL))) throw(Exception(errno, LOCATION, #FUNCTION_CALL " failed")); }

// some functions like malloc, calloc, fgets, fopen, dlopen return NULL on error. Doh!
#define THROW_ON_NULL(FUNCTION_CALL) { if (improbable(0 == (FUNCTION_CALL))) throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "%s", #FUNCTION_CALL " failed")); }

// some functions return 0 for error, like strtol and getpwname
#define THROW_ON_ZERO(FUNCTION_CALL) { if (improbable(0 == (FUNCTION_CALL))) throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "%s", #FUNCTION_CALL " failed 0")); }

// some functions like fwrite, fseek, remove, rename, feof return false on error
#define THROW_ON_FALSE(FUNCTION_CALL) { if (improbable((FUNCTION_CALL) == false)) throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "%s", #FUNCTION_CALL " failed")); }

// some functions like kill, socket, accept, listen, connect, unlink, waitpid return a non-zero to indicate an error
#define THROW_ON_ERRNO(FUNCTION_CALL) { if (improbable(0 != (FUNCTION_CALL))) throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "%s", #FUNCTION_CALL " failed")); }

#define THROW_ON_TRUE(FUNCTION_CALL) { if (improbable(0 != (FUNCTION_CALL))) throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "%s", #FUNCTION_CALL " must be 0 or false, and did not")); }

// this trys and catches (You supply the { action to be done }
#define IF_EXCEPTION(EXPRESSION) try { EXPRESSION; } catch(const std::exception& ex)

// this will append data to an exception and rethrow it. Sometimes only the catcher of an exception knows certain
// information, like failing to open a file, and it wants to inform the higher up callers of the filename so it can
// append that data, and rethrow the exception
#define IF_EXCEPTION_THROW(EXPRESSION) try { EXPRESSION; } catch(Exception& ex) { ex.Append( #EXPRESSION ); throw; }

/** Create an std::string that REALLY shows you the location in the code. */
#define ErrorLocation()  (std::string("function: \"") + __PRETTY_FUNCTION__ + "\" file: \"" + FileNameOnly(__FILE__) + ":" + Stringize(__LINE__))

/** Returns a string "reason: the last errno reason for an error" */
#define ErrorReason()  (errno != 0 ? std::string(" reason: \"") + std::strerror(errno) + "\"": "")

/** Returns location and reason. Really illuminates why and where a failure occured */
#define FullErrorInfo()  (ErrorReason() + ErrorLocation())

class Exception: public std::exception
{
    std::string StackTrace;
    std::string Location;
    std::string Explanation;
    std::string SystemErrorString;
    int SystemError;
    int ApplicationError;
    mutable std::string FullError;

    static int EnableStackTrace;

public:

    GETSET(std::string, Location);
    GETSET(std::string, Explanation);
    GETSET(std::string, SystemErrorString);
    GETSET(int, SystemError);
    GETSET(int, ApplicationError);
    GETSET_STATIC(int, EnableStackTrace);
    GETTER(std::string, StackTrace);

    void Append(const char* more_data);

    enum
    {
        ERRNO_IGNORE = -2, NO_SYSTEM_ERROR = -1, LOOKUP = 0
    };

    Exception(const char* location = LOCATION);

    Exception(int system_error, int application_error, const char* location, const char* explanation, ...)   __attribute__((format(printf, 5, 6)));;

    Exception(int error, const char* location, const char* explanation, ...)   __attribute__((format(printf, 4, 5)));;

    Exception(const char* location, const char* explanation_format, ...)  __attribute__((format(printf, 3, 4)));

    Exception(int error, const char* location, const Text& explanation);

    void Construct(const char* location, const char* explanation,
		   int system_error = Exception::LOOKUP, int application_error =
		   Exception::NO_SYSTEM_ERROR);

    std::string userwhat() const throw();

    std::string fullwhat() const throw();

    const char* what() const throw ();

    virtual ~Exception() throw ();
};

// Outside the normal Exception hierarchy. This is an intentional demand to exit so we only want it caught at the outermost scope of the program.
class ExitException
{
    std::string Reason;
    int Value;

public:
    ExitException(const char* reason, int value)
	: Reason(reason), Value(value)
    {
    }

    operator int() const { return Value; }

    operator const char*() const { return Reason.c_str(); }
};
