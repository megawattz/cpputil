#pragma once

#include <Text.h>
#include <DiskFileStream.h>
#include <StreamFactory.h>

class Logger
{
    struct timeval LastLog;

    volatile int LogLevel;
    int CurrentCount;
    Text Basepattern;
    Text Fillname;
    Text LogFormatDescriptionLine;

    int PeriodSize;
    int PeriodType;  // 0 = minutes, 1 = lines
    size_t Sequence;

    StreamPtr Log;

public:
    typedef std::map<int, Text> LASTLOG;

private:
    LASTLOG LastOfEachLog; // store the last instance of each log category;

    static const char* LogLevelNames[];

public:
    enum
    {
	ALWAYS,   // Always print message
	FATAL,    // Error that is probably fatal to program
	ERROR,    // Very bad condition that prevents some part of program from working
	IMPORTANT,// Not an error but a message that should always be seen
	WARN,     // Something you feel the user should know
	EXCEPT,   // An exception is thrown (not necessarily an error but often meaning something strange is going on)
	PERIODIC, // Logs that occur at periods of a minute or more.
	NORMAL,   // The default log level
	TIMING,   // Timing Statements
	UNEXP,    // Something happened, not an error, but which should rarely occur
	INFO,     // Information that is definitely not an error condition
	DEBUG,    // Information that only a programmer would be interested in
	TRACE,    // Show all logging messages
	LAST      // Placeholder
    };

    enum
    {
	UNLIMITED,
	SECONDS,
	LINES
    };

    void ClearLastOfEachLog();

    static const char* LevelName(const unsigned level) __attribute__ ((hot));

    static Text ExplainLevels();

    void CollectStrayLogs(const char* pattern);

    void setPeriod(const char* str);

    void CheckRotation(struct timeval* ts = 0);

    void setLogLevel(int level);

    GETTER(volatile int, LogLevel);
    GETTER(Text, Basepattern);
    GETSET(LASTLOG, LastOfEachLog);
    GETSET(timeval, LastLog)

    Text& setBasepattern(const Text& base_pattern);

    GETSET(Text, LogFormatDescriptionLine);
    GETSET(int, PeriodSize);
    GETSET(int, PeriodType);
    GETSET(Text, Fillname);

    Logger(const char* basename = "handle:2", const char* format = "");

    void Open(struct timeval* ts = 0);

    size_t write(int priority, const char* where, struct timeval* when, const char* text, size_t length);

    size_t Write(int priority, const char* where, struct timeval* when, const char* text, size_t length);

    size_t Write(int priority, const char* where, struct timeval* when, const Text& text);

    size_t Printf(int priority, const char* where, struct timeval* when, const char* format, ...) __attribute__((format(printf, 5, 6)));

    void Close(); // used for final cleanup
};
