/*
Copyright 2009 by Walt Howard
$Id: Exception.cc 2428 2012-08-14 15:33:13Z whoward $
*/

#include <Exception.h>
#include <StackTrace.h>

int Exception::EnableStackTrace = 0;

Exception::Exception(const char* location)
{
    Construct(location, "", Exception::NO_SYSTEM_ERROR, Exception::NO_SYSTEM_ERROR);
}


Exception::Exception(int system_error, int application_error, const char* location, const char* explanation_format, ...)
{
    char buffer[4096];

    va_list args;
    va_start(args, explanation_format);
    const int written = ::vsnprintf(buffer, sizeof(buffer) - 1, NO_NULL_STR(explanation_format), args);
    va_end(args);

    if (improbable(written < 0))
	strcpy(buffer, "Error writing error explanation");
    else
	buffer[LESSER(written, sizeof(buffer))] = '\0';

    Construct(location, buffer, system_error, application_error);
}

Exception::Exception(int error, const char* location, const char* explanation_format, ...)
{
    char buffer[4096];

    va_list args;
    va_start(args, explanation_format);
    const int written = ::vsnprintf(buffer, sizeof(buffer) - 1, explanation_format, args);
    va_end(args);

    if (improbable(written < 0))
	strcpy(buffer, "Error writing error explanation");
    else
	buffer[LESSER(written, sizeof(buffer))] = '\0';

    Construct(location, buffer, error, error);
}

Exception::Exception(const char* location, const char* explanation_format, ...)
{
    char buffer[4096];

    va_list args;
    va_start(args, explanation_format);
    const int written = ::vsnprintf(buffer, sizeof(buffer) - 1, explanation_format, args);
    va_end(args);

    if (improbable(written < 0))
	strcpy(buffer, "Error writing error explanation");
    else
	buffer[LESSER(written, sizeof(buffer)) ] = '\0';

    Construct(location, buffer);
}

void Exception::Construct(const char* location, const char* explanation, int system_error, int application_error)
{
    SystemError = 0;
    ApplicationError = 0;

    if (system_error == LOOKUP)
	SystemError = errno;

    if (system_error > LOOKUP)
	SystemError = system_error;

    if (SystemError > 0)
	SystemErrorString = NO_NULL_STR(::strerror(SystemError));

    Location = NO_NULL_STR(location);
    Explanation = NO_NULL_STR(explanation);

    if (EnableStackTrace)
	StackTrace = GetBacktrace();
}

Exception::Exception(int error, const char* location, const Text& explanation)
{
    Construct(location, explanation.c_str(), error, error);
}

std::string Exception::userwhat() const throw()
{
    return Explanation;
}

const char* Exception::what() const throw ()
{
    FullError = Explanation + " " + SystemErrorString;
    if (not Location.empty())
	FullError += " at:" + Location;
    return FullError.c_str();
}

std::string Exception::fullwhat() const throw ()
{
    FullError = Explanation + " " + SystemErrorString + " at:" + Location + "\n" + StackTrace;
    return FullError.c_str();
}

void Exception::Append(const char* more_data)
{
    (Explanation += "\n") += NO_NULL_STR(more_data);
}

Exception::~Exception() throw ()
{
}
