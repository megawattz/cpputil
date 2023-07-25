/*
Copyright 2009 by Walt Howard
$Id: Logger.h 2424 2012-08-13 19:07:09Z whoward $
*/

#pragma once

#include <Logger.h>

// Checks the log level BEFORE calling the function. Prevents the arguments from having to be constructed if they aren't going to even be used
//#define LOG(LOGGER, PRIORITY, FORMAT, ARGS...) if ((LOGGER->getLogLevel()) > (PRIORITY)) LOGGER->Printf((PRIORITY), LOCATION, time(0), FORMAT, ARGS)

/*
WHY USE MACROS FOR LOGGING?

Macros can expand __FILE__ __LINE__ __FUNCTION__ and __PRETTY_FUNCTION__ to tell you exactly where the log statement is
Macros can be defined away if you want to remove them. Even if you don't log something, calculating what IS to be logged, like creating a giant string, affects performance.

*/

// Use separate macros for each log level so they can be compiled OUT of the code individually.

#define LOGGER_LOG(PRIORITY, LOCATION, LOGGER, FORMAT, ARGS...) do { if (improbable(((LOGGER)->getLogLevel()) >= (PRIORITY))) (LOGGER)->Printf(PRIORITY, LOCATION, 0, FORMAT, ARGS); } while(false);

#ifdef FAST
#define LOG_DEBUG(X,Y,Z...) //
#define LOG_INFO(X,Y,Z...) //
#define LOG_TIMING(X,Y,Z...) //
   #warning "TRACE LOG_DEBUG and LOG_INFO compiled OUT"
#endif

#ifndef LOG_ALWAYS
   #define LOG_ALWAYS(LOGGER, FORMAT, ARGS...) LOGGER_LOG(Logger::ALWAYS, LOCATION, LOGGER, FORMAT, ARGS)
#endif

#ifndef LOG_FATAL
   #define LOG_FATAL(LOGGER, FORMAT, ARGS...) LOGGER_LOG(Logger::FATAL, LOCATION, LOGGER, FORMAT, ARGS)
#endif

#ifndef LOG_ERROR
   #define LOG_ERROR(LOGGER, FORMAT, ARGS...) LOGGER_LOG(Logger::ERROR, LOCATION, LOGGER, FORMAT, ARGS)
#endif

#ifndef LOG_IMPORTANT
   #define LOG_IMPORTANT(LOGGER, FORMAT, ARGS...) LOGGER_LOG(Logger::IMPORTANT, LOCATION, LOGGER, FORMAT, ARGS)
#endif

#ifndef LOG_WARN
   #define LOG_WARN(LOGGER, FORMAT, ARGS...) LOGGER_LOG(Logger::WARN, LOCATION, LOGGER, FORMAT, ARGS)
#endif

#ifndef LOG_EXCEPT
   #define LOG_EXCEPT(LOGGER, FORMAT, ARGS...) LOGGER_LOG(Logger::EXCEPT, LOCATION, LOGGER, FORMAT, ARGS)
#endif

#ifndef LOG_UNEXP
   #define LOG_UNEXP(LOGGER, FORMAT, ARGS...) LOGGER_LOG(Logger::UNEXP, LOCATION, LOGGER, FORMAT, ARGS)
#endif

#ifndef LOG_PERIODIC
   #define LOG_PERIODIC(LOGGER, FORMAT, ARGS...) LOGGER_LOG(Logger::PERIODIC, LOCATION, LOGGER, FORMAT, ARGS)
#endif

#ifndef LOG_NORMAL
   #define LOG_NORMAL(LOGGER, FORMAT, ARGS...) LOGGER_LOG(Logger::NORMAL, LOCATION, LOGGER, FORMAT, ARGS)
#endif

#ifndef LOG_TIMING
   #define LOG_TIMING(LOGGER, FORMAT, ARGS...) LOGGER_LOG(Logger::TIMING, LOCATION, LOGGER, FORMAT, ARGS)
#endif

#ifndef LOG_INFO
   #define LOG_INFO(LOGGER, FORMAT, ARGS...) LOGGER_LOG(Logger::INFO, LOCATION, LOGGER, FORMAT, ARGS)
#endif

#ifndef LOG_DEBUG
   #define LOG_DEBUG(LOGGER, FORMAT, ARGS...) LOGGER_LOG(Logger::DEBUG, LOCATION, LOGGER, FORMAT, ARGS)
#endif
