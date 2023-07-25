/*
Copyright 2009 by Walt Howard
$Id: Logger.cc 2428 2012-08-14 15:33:13Z whoward $
*/

#include <Misc.h>
#include <Logger.h>
#include <Exception.h>
#include <StreamFactory.h>
#include <string.h>
#include <dirent.h>

const char* Logger::LogLevelNames[] =
{
    "Always",
    "Fatal",
    "Error",
    "Important",
    "Warning",
    "Exception",
    "Periodic",
    "Normal",
    "Timing",
    "Unexpected",
    "Information",
    "Debug",
    "Trace",
    "All",
    0
};

void Logger::ClearLastOfEachLog()
{
    LastOfEachLog.clear();
}


const char* Logger::LevelName(const unsigned level)
{
    return LogLevelNames[Clip(0, level, sizeof LogLevelNames - 1)];
}


Text Logger::ExplainLevels()
{
    Text levels;
    for(int i(ALWAYS); i < LAST; ++i)
	levels += StringPrintf(0, "%d-%s ", i, LevelName(i));
    return levels;
}


Logger::Logger(const char* basepattern, const char* format)
    : CurrentCount(-1), Basepattern(basepattern), LogFormatDescriptionLine(format),
      PeriodSize(15 * 60), PeriodType(SECONDS),
      LogLevel(NORMAL), Sequence(0), LastLog({0, 0})
{
}


Text& Logger::setBasepattern(const Text& base_pattern)
{
    Basepattern = base_pattern;
    LastLog.tv_sec = 0;
    LastLog.tv_usec = 0;
    //  Open();
    return Basepattern;
}


void Logger::CollectStrayLogs(const char* pattern)
{
    //Rename all ".filling" files to ".log" files. Necessary when bringing up a server that crashed.

    bool disk(false);
    char* urlstr = strdupa(pattern);

    // a disk file or no protocol specified means a disk file
    if (strstr(urlstr, "file:") == urlstr)
	disk = true;

    if (strcspn(urlstr, ":") > 12 or not strchr(urlstr, ':'))
	disk = true;

    if (strncmp(urlstr, "/dev/", 5) == 0)
	disk = false;

    if (not disk)
	return; // no need to collect old files, logging is not to file

    char* work = strdupa(ReplaceSystemVariables(url_resource(urlstr)));

    // see if the files are being placed in another directory
    char* w = strrchr(work, '/');

    if (w)
	*w = '\0';

    Text directory(w ? work : ".");

    int count(0);

    DIR* dir = opendir(directory);

    if (not dir)
	throw(Exception(LOCATION, "Error Opening Directory: \"%s\"", directory.c_str()));

    int len = strlen(".filling");

    for(struct dirent* file = readdir(dir); file; file = readdir(dir))
    {
	// does it contain .filling?
	char* f = strstr(file->d_name, ".filling");
	if (not f)
	    continue;

	// skip if the .filling wasn't the last part of the file name
	if (*(f + len) != '\0')
	    continue;

	Text oldname(StringPrintf(0, "%s/%s", directory.c_str(), file->d_name));

	Text newname(ChangeFileSuffix(oldname.c_str(), ".log"));

	if (-1 == rename(oldname.c_str(), newname.c_str()))
	{
	    closedir(dir);
	    throw(Exception(LOCATION, "Error renaming %s to %s", oldname.c_str(), newname.c_str()));
	}
    }

    closedir(dir);
}


void Logger::Close()
{
    Log->close();

    if (StrBackMatch(Basepattern.c_str(), "@F"))
    {
        Text finalname = ReplaceAll(Fillname, ".filling", ".log");

        // This is not a fatal error. Another thread or process may have already renamed the file.
        if (Fillname != finalname)
            if (-1 == rename(Fillname.c_str(), finalname.c_str()))
            {
                std::cerr << StringPrintf(0, "Unable to rename %s to %s error %s (not necessarily fatal). Will continue to append to: %s\n",
                                          Fillname.c_str(), finalname.c_str(), StrError(errno).c_str(), Fillname.c_str());
            }
    }
}


void Logger::Open(struct timeval* ts)
{
    struct timeval when;

    if (ts == 0)
	when = GetTimeOfDay();
    else
	when = *ts;

    Text url = Basepattern;

    bool disk(false);

    /*
      Some logic here to make it so that ONLY disk files are rotated. We can't rotate syslog udp packets etc.
    */

    // some "files" are actually standard handles, (they cannot be renamed)
    if (strcasecmp(url.c_str(), "stderr") == 0 or strcasecmp(url.c_str(), "stdout") == 0)
    {
	url = "handle:" + url;
    }

    const char* urlstr = url.c_str();

    // a disk file or no protocol specified means a disk file
    if (strstr(urlstr, "file:") == urlstr)
	disk = true;

    if (strcspn(urlstr, ":") > 12 or not strchr(urlstr, ':'))
    {
	// no protocol specifier, assume "file://"
	url = "file://" + url;
	disk = true;
    }

    urlstr = url.c_str();

    // some "files" don't act like disk files (they cannot be renamed)
    if (strstr(urlstr, "file:///dev") == urlstr)
	disk = false;

    if (disk)
    {
	if (not Fillname.empty())
	{
	    Text finalname(Fillname);

	    if (StrBackMatch(Basepattern.c_str(), "@F"))
	    {
		finalname = ReplaceAll(Fillname, ".filling", ".log");

		// This is not a fatal error. Another thread or process may have already renamed the file.
		if (Fillname != finalname and -1 == rename(Fillname.c_str(), finalname.c_str()))
		{
		    std::cerr << StringPrintf(0, "Unable to rename %s to %s error %s (not necessarily fatal). Will continue to append to: %s\n",
					      Fillname.c_str(), finalname.c_str(), StrError(errno).c_str(), Fillname.c_str());
		}
	    }
	}

	Fillname = url_resource(ReplaceSystemVariables(Basepattern.c_str()).c_str());

	CurrentCount = 0;

	url = "file://" + Fillname;
    }
    else
    {
	setPeriod("U");
    }

    try
    {
	Log = StreamFactory(url.c_str(), "APPEND CREATE WRONLY TIMEOUT=5");
	Log->open();
    }
    catch(const std::exception& ex)
    {
	// If an error happens while writing to logs, change log to standard error
	Basepattern = "handle:stderr";
	Open();
	write(Logger::ERROR, LOCATION, &when, ex.what(), strlen(ex.what()));
    }

    if (LogFormatDescriptionLine)
    {
	Text expanded = ReplaceSystemVariables(LogFormatDescriptionLine);
        Log->Printf("%s\n", expanded.c_str());
    }

    LastLog = when;

    Sequence = 0;
}

void Logger::CheckRotation(struct timeval* ts)
{
    struct timeval when;

    if (ts == 0)
	when = GetTimeOfDay();
    else
	when = *ts;

    if (improbable(not Log.get()))
    {
	Open();
    }
    else if (PeriodType == SECONDS)
    {
	int last_period = LastLog.tv_sec / PeriodSize;
	int now_period = when.tv_sec / PeriodSize;
	if ( last_period < now_period )
	{
	    Open();
	}
    }
    else if (PeriodType == LINES)
    {
	if ((CurrentCount != -1 and ++CurrentCount >= PeriodSize))
	    Open();
    }
}


size_t Logger::write(int priority, const char* where, struct timeval* ts, const char* text, size_t length)
{
    size_t rval = 0;

    if (probable(priority != ALWAYS) and probable(priority > LogLevel))
	return rval;

    if (improbable(length == 0))
	length = strlen(text);

    struct timeval when;

    if (ts == 0)
	when = GetTimeOfDay();
    else
	when = *ts;

    CheckRotation(&when);

    if (priority == ALWAYS)
    {
        LastLog = when;
        rval = Log->Printf("%s\n", text);
        return rval;
    }

    char timebuffer[40];
    THROW_ON_ZERO(strftime(timebuffer, sizeof(timebuffer), "%Y-%m-%d_%H:%M:%S", gmtime(&when.tv_sec)));

    char logbuffer[1024];

    Text temp;

    // hanging indent on multiline logs
    if (improbable(strchr(text, '\n')))
    {
	temp = ReplaceAll(text, "\n", "\n  \t");
	text = temp.c_str();
    }

    if (probable(LogLevel < UNEXP))
    {
	snprintf(logbuffer, sizeof(logbuffer) - 1, "--\t%s\t%s\n", timebuffer, text);
    }
    else if (probable(LogLevel < DEBUG))
    {
	snprintf(logbuffer, sizeof(logbuffer) - 1, "--\t%s\t%4.4x\t%s\t%s\t%s\n", timebuffer, GetThreadId(), NO_NULL_STR(where), LevelName(priority), text);
    }
    else
    {
	snprintf(logbuffer, sizeof(logbuffer) - 1, "--\t%s.%6.6ld\t%9lld\t%5ld\t%4.4x\t%s\t%s\t%s\n", timebuffer, when.tv_usec, 
                 TimeOfDayDiff(LastLog, when), Sequence, GetThreadId(), NO_NULL_STR(where), LevelName(priority), text);
	++Sequence;
    }

    if (Logger::LogLevel >= Logger::INFO)
	LastOfEachLog[priority] = logbuffer;

    rval = Log->writeString(logbuffer);

    LastLog = when;

    return rval;
}


size_t Logger::Write(int priority, const char* where, struct timeval* when, const char* text, size_t length)
{
    return write(priority, where, when, text, length);
}


size_t Logger::Write(int priority, const char* where, struct timeval* when, const Text& text)
{
    if (priority != ALWAYS and probable(priority > LogLevel))
	return 0;

    return Write(priority, where, when, text.c_str(), text.size());
}


size_t Logger::Printf(int priority, const char* where, struct timeval* when, const char* format, ...)
{
    if (probable(priority != ALWAYS) and probable(priority > LogLevel))
	return 0;

    char buffer[66000];

    va_list args;
    va_start(args, format);
    const int written = ::vsnprintf(buffer, sizeof(buffer) - 1, format, args);
    va_end(args);

    try
    {
	return write(priority, where, when, buffer, written);
    }
    catch(const std::exception& ex)
    {
	// If an error happens while writing to logs, change log to standard error
	Basepattern = "handle:stderr";
	Open();
	write(Logger::ERROR, LOCATION, when, ex.what(), strlen(ex.what()));
    }

    return 0;
}


void Logger::setLogLevel(int level)
{
    timeval now;
    gettimeofday(&now, NULL);
    LogLevel = level;
}

void Logger::setPeriod(const char* str)
{
    int value = atoi(str);

    switch(LastChar(str))
    {

    case 'U':
    case 'u':
	PeriodType = UNLIMITED;
	break;

    case 'l':
    case 'L':
	PeriodType = LINES;
	PeriodSize = std::min(30000, value);
	break;

    case 'w':
    case 'W': value *= 7;
	// FALL THROUGH!

    case 'd':
    case 'D': value *= 24;
	// FALL THROUGH!

    case 'h':
    case 'H': value *= 60;
	// FALL THROUGH!

    case 'm':
    case 'M': value *= 60;
	// FALL THROUGH!

    case 's':
    case 'S':
	PeriodType = SECONDS;
	PeriodSize = value;
	break;

    default:
	throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Invalid period string: %s", str));
    }
}
