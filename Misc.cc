/*
Copyright 2009 by Walt Howard
$Id: Misc.cc 2428 2012-08-14 15:33:13Z whoward $
*/

#include <Misc.h>
#include <Text.h>
#include <Exception.h>
#include <map>
#include <vector>
#include <alloca.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <dirent.h>
#include <signal.h>
#include <Enhanced.h>

/**
 This is intended as a temporary holding area for these functions. When several of them fit the same category,
 create a separate file for them.
 */

Text StringPrintf(int, const char *format, ...)
{
    size_t bufsize(1024);

    for (;;)
    {
        char * const buffer(reinterpret_cast<char *> (::alloca(bufsize)));
        va_list args;
        va_start(args, format);
        const int written = ::vsnprintf(buffer, bufsize, format, args);
        //va_end(args);

        // If there was insufficient room, set size to what is needed and try again
        // if vsnprintf fails it returns the number of bytes NEEDED so we can try again and succeed
        if (improbable(written >= static_cast<int> (bufsize)))
        {
	    va_end(args);
            bufsize = written + 1;
            continue;
        }

        if (improbable(written < 0))
            throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "in StringUtil::Format: Error formatting Text object."));

        /**
         * Returning a local pointer? Not really. Text will get constructed from this (return
         * value optimization).
         */
        return buffer;
    }
}

Text StringPrintf(const char *format, ...)
{
    size_t bufsize(1024);

    for (;;)
    {
        char * const buffer(reinterpret_cast<char *> (::alloca(bufsize)));
        va_list args;
        va_start(args, format);
        const int written = ::vsnprintf(buffer, bufsize, format, args);
        //va_end(args);

        // If there was insufficient room, set size to what is needed and try again
        // if vsnprintf fails it returns the number of bytes NEEDED so we can try again and succeed
        if (improbable(written >= static_cast<int> (bufsize)))
        {
	    va_end(args);
            bufsize = written + 1;
            continue;
        }

        if (improbable(written < 0))
            throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "in StringUtil::Format: Error formatting Text object."));

        /**
         * Returning a local pointer? Not really. Text will get constructed from this (return
         * value optimization).
         */
        return buffer;
    }
}

/** @brief  Construct a string using a printf style format into a user supplied buffer, and return that buffer
 */
char* Sprintf(char* buffer, int size, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    const int written = ::vsnprintf(buffer, size, format, args);
    va_end(args);

    if (improbable(written >= static_cast<int>(size)))
	throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "String of size: %d cannot fit in buffer of size %d", written, size));

    if (improbable(written < 0))
	throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "in StringPrintf: Error formatting Text object."));

    return buffer;
}

std::list<Text> SplitOnChars(const char* source, const char* splitters)
{
    std::list<Text> elements;
    // We need a non-const copy of the source. strtok alters its source string.
    char *work_string = strdupa(source);
    for (const char* item = std::strtok(work_string, splitters); item != NULL; item
            = std::strtok(NULL, splitters))
        elements.push_back(item);
    return elements;
}

Text GetProgCommandLine()
{
    Text commandline; // If the read fails, "Unknown" will be returned
    std::ifstream self_info("/proc/self/cmdline");
    if (not self_info.good())
        return StringPrintf(0, "ProcessId:%d", ::getpid());
    self_info >> commandline;
    return ProcCmdlineToString(commandline);
}

Text GetProgName()
{
    char* execname = strdupa(GetProgCommandLine().c_str()); // Grab up to first NULL.
    if (char* slash = std::strrchr(execname, '/'))
        execname = slash + 1;
    char* first = strtok(execname, " \t");
    return first;
}

Text GetProgPath()
{
    char fullpath[PATH_MAX + 1];
    int length = readlink("/proc/self/exe", fullpath, PATH_MAX);
    THROW_ON_ERROR(length);
    fullpath[length] = '\0';
    return fullpath;
}

uid_t GetExecOwner()
{
    return Stat(GetProgPath()).st_uid;
}

gid_t GetExecGroup()
{
    return Stat(GetProgPath()).st_gid;
}

Text ScanfString(const char* source, const char* scanf_format)
{
    char buffer[8192] = "";
    sscanf(source, scanf_format, buffer);
    return buffer;
}

const char* SelectString(const unsigned zero_based_selection, const char* string0, ...)
{
    va_list args;
    va_start(args, string0);

    if (zero_based_selection == 0)
	return string0;

    const char* choice(NULL);

    for (unsigned which(1); which <= zero_based_selection; ++which)
    {
        choice = va_arg(args, const char* );
        if (choice == NULL or which > zero_based_selection)
        {
            va_end(args);
            return ""; // Could not find value, return empty string.
        }
        if (which == zero_based_selection)
        { // Found a string at index? Return it.
            va_end(args);
            return choice;
        }
    }
    return "";
}

int IndexOfString(const char* search_for_string,
        const char* strings_to_search_in_0, ...)
{
    va_list args;
    va_start(args, strings_to_search_in_0);
    int index(0);

    for (const char* current = strings_to_search_in_0; current != NULL; current
            = va_arg(args, const char*))
    {
        if (::strcasecmp(current, search_for_string) == 0)
        {
            va_end(args);
            return index;
        }
        ++index;
    }
    va_end(args);
    return -1;
}

Text FileNameOnly(const char* path)
{
    const char* p = std::strrchr(path, '/');

    // if we found a /, increment one past and return that.
    return p ? p + 1 : path;
}

Text FileNameOnly(const Text& path)
{
    return FileNameOnly(path.c_str());
}

void WriteToStderr(const Text& message)
{
    int i = ::write(STDERR_FILENO, message.c_str(), message.length());
}

void WriteToStdout(const Text& message)
{
    int i = ::write(STDOUT_FILENO, message.c_str(), message.length());
}

/** Replace all instances of replacee with replaceor preventing recursion */
Text ReplaceAll(const Text& source, const Text& replacee,
        const Text& replaceor)
{
    size_t new_size_max;

    // is our replaceor bigger than our replacee? Then the string will grow, and the maximum
    // growth will be if the entire string consists of replacees.
    if (replaceor.size() > replacee.size())
	new_size_max = (source.size() * replaceor.size()) / replacee.size();
    else
	new_size_max = source.size(); // otherwise it will shrink so same size is ok.

    char buffer[new_size_max + 1];

    char* insert = buffer;

    const char* src = source.c_str();

    for( const char* pos = strstr(src, replacee.c_str()); pos; pos = strstr(src, replacee.c_str()))
    {
	size_t jump(pos - src); // how much non-replace did we jump?
	strncpy(insert, src, jump); // copy all the non-replace directly to output buffer
	src += jump; // increment src
	insert += jump; // increment the output buffer
	strncpy(insert, replaceor.c_str(), replaceor.size()); // add the replaceor
	insert += replaceor.size(); // move the insert point past the replaceor.
	src += replacee.size(); // skip past the replacee in the source string
        pos = strstr(src, replacee.c_str());  // find another replacee
    }

    // any remaining source that didn't need replacement needs to be appended.
    strcpy(insert, src);

    return Text(buffer);
}

Text ProcCmdlineToString(const Text& source)
{
    Text work(source);
    for (Text::iterator ch(work.begin()); ch != work.end(); ++ch)
        if (*ch == '\0')
            *ch = ' ';
    return work;
}


Text GetProgAbsolutePath()
{
    char buffer[PATH_MAX] = "";
    int length = ::readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (-1 == length)
        return StringPrintf(0, "ProcessId:%d", ::getpid());
    buffer[length] = '\0';
    return buffer;
}

std::vector<int> HasRead(const unsigned timeout_milliseconds,
        const std::vector<int>& fds_to_check)
{
    for (;;)
    {
        std::vector<int> readies; // will contain descriptors that are readable

        fd_set read_set;
        FD_ZERO(&read_set);

        int highest_descriptor(-1);

        for (std::vector<int>::const_iterator fd(fds_to_check.begin()); fd != fds_to_check.end(); ++fd)
        {
            FD_SET(*fd, &read_set);
            highest_descriptor = highest_descriptor > *fd ? highest_descriptor : *fd;
        }

        // Convert milliseconds into seconds and remaining micro seconds.
        struct timeval timeout = { timeout_milliseconds / 1000, (timeout_milliseconds % 1000) * 1000 };

        switch (::select(highest_descriptor + 1, &read_set, NULL, NULL, &timeout))
        {
	case -1:
	    if (errno != EINTR)
		throw(Exception(LOCATION, "Select error."));
	    break; // try again

	case 0:
	    return readies; // readies of course is empty here. We're returning an empty vector.

	default:
	{
	    // If some of the descriptors are read ready, fill the return vector with them.
	    for (std::vector<int>::const_iterator fd(fds_to_check.begin()); fd
		     != fds_to_check.end(); ++fd)
	    {
		if (FD_ISSET(*fd, &read_set))
		    readies.push_back(*fd);
	    }

	    return readies;
	}
        }
    }
}

Text ExtractClassNameFromPrettyFunction(const char* pretty_function)
{
    const char* start = strrchr(pretty_function, '=') + 2;

    const char* end = strchr(start, ']');

    return Text(start, end - start);
}

Text ShortenizeClassName(const char* fullname)
{
    const char* end = strchr(fullname, '<');

    if (improbable(end != 0))
	return Text(fullname, end - fullname);

    return fullname;
}

Text ExtractShortClassNameFromPrettyFunction(const char* pretty_function)
{
    const char* start = strrchr(pretty_function, '=') + 2;

    const char* end = strchr(start, ']');

    const char* end2 = strchr(start, '<');

    if (improbable(end2 != 0))
	return Text(start, end2 - start);

    return Text(start, end - start);
}

void MakeDirectory(const char* path, mode_t mode)
{
    int rval = ::mkdir(path, mode);
    if (rval == 0)
        return;

    if (errno != EEXIST)
        throw(Exception(LOCATION, "Creating directory: %s", path));

    struct stat data;
    THROW_ON_ERROR(::stat(path, &data));

    if (not S_ISDIR(data.st_mode))
        throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "%s already exists and is not a directory.", path));
}

int FileType(const char* filename)
{
  struct stat data;
  if (-1 == ::stat(filename, &data))
    return 0;

  return data.st_mode;
}

long long DiffTime(const struct timeval& earlier, const struct timeval& later)
{
    suseconds_t mics = later.tv_usec - earlier.tv_usec;
    time_t secs = later.tv_sec - earlier.tv_sec;
    return (secs * 1000000) + mics;
}

long long DiffTimeSpec(const struct timespec& earlier, const struct timespec& later)
{
    suseconds_t mics = later.tv_nsec - earlier.tv_nsec;
    time_t secs = later.tv_sec - earlier.tv_sec;
    long long diff = (secs * 1000000000) + mics;
    return diff;
}

double DiffTimeSeconds(const struct timeval& earlier, const struct timeval& later)
{
    long long temp = DiffTime(earlier, later);
    return (double)temp / 1000000.0;
}

Text TimeStringLocal(const char* format, time_t tTime)
{
    char buffer[1024];
    struct tm result;
    if (0 == strftime(buffer, sizeof(buffer) - 1, format, localtime_r(&tTime, &result)))
	throw Exception(LOCATION, "strftime failed: %s", format);
    return buffer;
}

Text TimeStringGmt(const char* format, time_t tTime)
{
    char buffer[1024];
    struct tm result;
    THROW_ON_ZERO(strftime(buffer, sizeof(buffer) - 1, format, gmtime_r(&tTime, &result)));
    return buffer;
}


Text StrError(int errnum)
{
    char buf[1024];
    strerror_r(errnum, buf, sizeof(buf) - 1);
    return buf;
}

Text ExpandSpecialCharacters(const char* str)
{
    if (not str)
        return "";

    char buffer[strlen(str) * 4];
    char* o = buffer;

    for(const char* p = str; *p; ++p)
    {
        if (*p != '\\')
        {
            *o++ = *p;
            continue;
        }

        p++;

        switch(*p)
        {
        case '\\':
            *o++ = '\\';
            break;

        case 'r':
            *o++ = '\r';
            break;

        case 'n':
            *o++ = '\n';
            break;

        case 't':
            *o++ = 't';
            break;

        case 'b':
            *o++ = '\b';
            break;

        case 'f':
            *o++ = '\f';
            break;

        case 'v':
            *o++ = '\v';
            break;

        case '0':
            *o++ = '\0';
            break;

        case 'x':
        {
            ++p;
            int y = 0;
            sscanf(p, "%2x", &y);
            p += 2;
            *o++ = (char)y;
        }
        break;

        default:
            *o++ = *p;
            break;

        }
    }

    *o = '\0';
    return buffer;
}


// replace various % symbols with important system informations
Text ReplaceSystemVariables(const char* str)
{
    if (not str or not *str)
	return "";

    Text temp = TimeStringLocal(str, time(0));

    char buffer[1024] = "";
    if (strstr(temp.c_str(), "@H"))
    {
	gethostname(buffer, sizeof(buffer) - 1);
	temp = ReplaceAll(temp, "@H", buffer);
    }

    // THIS MUST NOT BE MOVED TO BEFORE THE PREVIOUS gethostname
    if (strstr(temp.c_str(), "@C")) // Data center, first 3 letter of host
    {
        if (not buffer[0])
            gethostname(buffer, sizeof(buffer) - 1);
        buffer[3] = '\0';
	temp = ReplaceAll(temp, "@C", buffer);
    }

    if (strstr(temp.c_str(), "@P"))
    {
	temp = ReplaceAll(temp, "@P", ToString(getpid()));
    }

    if (strstr(temp.c_str(), "@F"))
    {
	temp = ReplaceAll(temp, "@F", ".filling");
    }

    if (strstr(temp.c_str(), "@T"))
    {
	// this gets the thread id but is no implemented in glibc as a c function so you have to call it this way.
	pid_t thread_id;
	THROW_ON_ERROR(thread_id = syscall(SYS_gettid));
	temp = ReplaceAll(temp, "@T", ToString(thread_id));
    }

    // environment variable expansion
    while (const char* p = strstr(temp.c_str(), "$("))
    {
	const char* end = strstr(p + 2, ")");
	if (not end)
	    break;

	Text replacee(p, end - p + 1);

	Text var(p + 2, end - p - 2);

	const char* env = getenv(var.c_str());
	if (not env)
	    env = "";

	temp = ReplaceAll(temp, replacee, env);
    };

    // shell special characters
    if (const char* p = strstr(temp.c_str(), "~"))
    {
	const char* env = getenv("HOME");
	if (not env)
	    env = "";

	temp = ReplaceAll(temp, "~", env);
    };

    return temp;
}

char LastChar(const char* str)
{
    int c = strlen(str);

    if (improbable(c < 1))
	return '\0';

    return str[c - 1];
}

const char* ReverseFindChar(const char* from, const char* haystack_start, char needle)
{
    for(const char* p = from; p >= haystack_start; --p)
	if (improbable(*p == needle))
	    return p;

    return 0;
}

struct stat Stat(const char* filename)
{
    struct stat data;
    if (improbable(-1 == stat(NO_NULL_STR(filename), &data)))
	throw Exception(LOCATION, "Cannot stat file: %s", NO_NULL_STR(filename));
    return data;
}

bool Exists(const char* filename)
{
    struct stat data;
    if (improbable(-1 == stat(NO_NULL_STR(filename), &data)))
	return false;
    return true;
}

int OpenFileCount()
{
    int count(0);

    DIR* dir = opendir("/proc/self/fd");
    if (not dir)
	throw(Exception(LOCATION, "Error Opening Directory: /proc/self/fd"));

    for(struct dirent* file = readdir(dir); file; file = readdir(dir))
	++count;

    closedir(dir);

    return count;
}

int Strpcpy(const char* source, char* target, size_t length)
{
    if (improbable(length == 0))
	length = std::min(length, strlen(source));
    strncpy(target, source, length);
    target[length] = '\0'; // ensure null termination
    return length;
}

Text HumanTimeDiff(const time_t& earlier, const time_t& later)
{
    long total = labs(later - earlier);

    div_t result;

    result = div(total, 86400);
    int days = result.quot;

    result = div(result.rem, 3600);
    int hours = result.quot;

    result = div(result.rem, 60);
    int minutes = result.quot;

    int seconds = result.rem;

    return StringPrintf(0, "%dd%dh%dm%ds", days, hours, minutes, seconds);
}


char* TranslateChar(char* target, const char from, const char to)
{
    for(char* c = target; *c; ++c)
    {
	if (improbable(*c == from))
	    *c = to;
    }
    
    return target;
}


Text Base64Decode( const char* base64string )
{
    char* input = strdupa(base64string);
    char* original = input;

    // undo web safe substitutions before decoding
    TranslateChar(input, '-', '+');
    TranslateChar(input, '_', '/');

    /*
      static const char basis_64[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    */

    static const char index_64[256] = {
        -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,62, -1,-1,-1,63,
	52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1,-1,-1,-1,
	-1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
	15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
	-1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
	41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    };

#define CHAR64(c)  (index_64[(unsigned char)(c)])

    int len = 0;
    unsigned char *output = (unsigned char *)input;
    int c1, c2, c3, c4;

    while (*input and *input != '=')
    {
        c1 = *input++;

        if (CHAR64(c1) == -1)
            throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Base64 Decode Failed"));

        c2 = *input++;

        if (CHAR64(c2) == -1)
            throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Base64 Decode Failed"));

        c3 = *input++;

	if (c3 != '=' && CHAR64(c3) == -1)
	    throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Base64 Decode Failed"));

        c4 = *input++;

        if (c4 != '=' && CHAR64(c4) == -1)
	    throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Base64 Decode Failed"));

        *output++ = (CHAR64(c1) << 2) | (CHAR64(c2) >> 4);

        ++len;

        if (c3 == '=')
            break;

        *output++ = ((CHAR64(c2) << 4) & 0xf0) | (CHAR64(c3) >> 2);

        ++len;

        if (c4 == '=')
            break;

        *output++ = ((CHAR64(c3) << 6) & 0xc0) | CHAR64(c4);

        ++len;
    }
    *output = '\0';

    return original;
}


char* Strlwr(char* str)
{
    for(char* p = str; *p; ++p)
	*p = tolower(*p);
    return str;
}


int Strpncpy(const char* source, char* target, int length)
{
    strncpy(target, source, length);
    return length;
}


char* Strtcpy(const char* source, char* target, size_t length)
{
    if (improbable(length == 0))
	length = strlen(source);
    strncpy(target, source, length);
    target[length] = '\0';
    return target;
}


pid_t GetThreadId()
{
    return syscall(SYS_gettid);
}


const char* StrBackMatch(const char* container, const char* contained)
{
    const char* hit = strstr(container, contained);

    if (not hit)
	return 0;

    // does the bigger string end exactly where the smaller string fits in?
    if (*(hit + strlen(contained)) == '\0')
	return hit;

    return 0;
}


const char* StrWhichRev(const char* haystack, char find, int which)
{
    const char* end = haystack + strlen(haystack);
    const char* walker = end;

    for(int i = which; i > 0; --i)
    {
	while (walker-- > haystack)
	{
	    if (*walker == find)
		break;
	}
    }

    return walker;
}


Text GetCookieDomain(const char* hostname)
{
    char* workname = strdupa(hostname);

    if (char* colon = strchr(workname, ':'))
	*colon = '\0';

    return StrWhichRev(workname, '.', 2);
}

void InterruptSystemCall(int signal)
{
    // does nothing but serves as a placeholder to interrupt a system call
}

void SetupSignalHandlers(void (*default_handler)(int), bool core_dump)
{
    struct sigaction terminate_action =  { default_handler, 0, 0, SA_RESETHAND, 0 };
    struct sigaction ignore_action=   { SIG_IGN, 0, 0, SA_RESETHAND, 0 };
    struct sigaction interrupt_system_call =  { InterruptSystemCall, 0, 0, SA_RESETHAND, 0 };

    THROW_ON_ERROR(sigaction(SIGUSR1,  &interrupt_system_call, 0));
    THROW_ON_ERROR(sigaction(SIGALRM,  &interrupt_system_call, 0));

    THROW_ON_ERROR(sigaction(SIGPIPE,  &ignore_action, 0));
    THROW_ON_ERROR(sigaction(SIGHUP, &ignore_action, 0));

    // don't trap SIGABORT at all

    THROW_ON_ERROR(sigaction(SIGINT, &terminate_action, 0));
    THROW_ON_ERROR(sigaction(SIGBUS, &terminate_action, 0));
    THROW_ON_ERROR(sigaction(SIGILL, &terminate_action, 0));
    THROW_ON_ERROR(sigaction(SIGQUIT, &terminate_action, 0));
    THROW_ON_ERROR(sigaction(SIGSEGV, &terminate_action, 0));
    THROW_ON_ERROR(sigaction(SIGSYS, &terminate_action, 0));
    THROW_ON_ERROR(sigaction(SIGTERM, &terminate_action, 0));
    THROW_ON_ERROR(sigaction(SIGXCPU, &terminate_action, 0));
    THROW_ON_ERROR(sigaction(SIGXFSZ, &terminate_action, 0));
}

Text GetHostname()
{
    char buffer[200];
    THROW_ON_ERROR(gethostname(buffer, sizeof(buffer) - 1));
    return buffer;
}

Text Getcwd()
{
    char buffer[PATH_MAX + 1];
    return getcwd(buffer, sizeof(buffer) - 1);
}

void Chdir(const char* directory)
{
    THROW_ON_NULL(directory);
    if (-1 == chdir(directory))
        throw Exception(LOCATION, "Could not Chdir to %s", directory);
}

Text ChangeFileSuffix(const char* filename, const char* new_suffix_with_period)
{
    THROW_ON_NULL(filename);
    THROW_ON_NULL(new_suffix_with_period);

    char* newname = strdupa(filename);

    if (char* p = strrchr(newname, '.'))
	*p = '\0';

    return Text(newname) + Text(new_suffix_with_period);
}

Text PrefixFile(const char* filename, const char* prefix)
{
    THROW_ON_NULL(filename);
    THROW_ON_NULL(prefix);
    char* newname = strndupa(filename, PATH_MAX);

    if (char* p = strrchr(newname, '/'))
    {
	*p++ = '\0';
	return Text(newname) + '/' + prefix + p;
    }

    return Text(prefix) + filename;
}

Text DecodeHttp(const char* encoded_http_request)
{
    // allocate room for the returned string on the stack.
    char* buffer = reinterpret_cast<char*>(alloca(strlen(encoded_http_request) + 4));
    char* insert = buffer;

    static const char* digits("0123456789ABCDEF");

    for(const char* p = encoded_http_request; *p; ++p)
    {
	if (*p != '%') // a normal, non escaped character?
	{
	    *insert++ = *p;
	    continue;
	}

	// first letter, 16s
	++p;
	if (!*p) // a '%' followed by '\0'. Not cool.
	    throw Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Invalid HTTP encoding: %s", encoded_http_request);
	const char* digit = strchr(digits, toupper(*p));
	if (not digit)
	    throw Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Invalid hex digit in HTTP encoding: %s", encoded_http_request);
	*insert = ((digit - digits) * 16);

	// second letter, units
	++p;
	if (!*p) // a '%' followed by only a single digit?'\0'. Not cool.
	    throw Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Invalid HTTP encoding: %s", encoded_http_request);
	digit = strchr(digits, toupper(*p));
	if (not digit)
	    throw Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Invalid hex digit in HTTP encoding: %s", encoded_http_request);
	*insert++ = *insert + (digit - digits);
    }

    *insert = '\0';
    return buffer;
}

Text NumberLines(const char* text, int start)
{
    Enhanced<std::vector<Text>> lines(text, "\r\n");

    Text processed;
    int number(start);

    for(Enhanced<std::vector<Text>>::const_iterator i(lines.begin()); i != lines.end(); ++i)
    {
	processed.append(StringPrintf(0, "%3d) %s\n", number++, i->c_str()));
    }

    return processed;
}

Text FirstFound(const char* search_in_string, const char* search_for_strings, ...)
{
    va_list args;
    va_start(args, search_for_strings);

    const char* pos(0);
    const char* found(0);

    for (const char* current = search_for_strings; probable(current != NULL); current = va_arg(args, const char*))
    {
        const char* find = strstr(search_in_string, current);
	if (not find)
	    continue;

	if ((find < pos) or (not pos))
        {
	    pos = find;       // where in the string
	    found = current;  // what was found;
        }
    }
    va_end(args);
    return found;
}

bool InString(const char* haystack, const char* needle)
{
    THROW_ON_NULL(haystack);
    THROW_ON_NULL(needle);

    const char* in = strstr(haystack, needle);
    if (not in)
	return false;

    if (*(in-1) != *haystack)  // first character of haystack is considered the delimiter
	return false;

    if (*(in + strlen(needle)) != *haystack)
	return false;

    return true;
}

Text FileBaseName(const char* filename)
{
    char* workfile = strdupa(filename ? filename : "");

    if (char* p = strrchr(workfile, '.'))
	*p = '\0';

    return workfile;
}

Text ExitReason(int status)
{
    Text reason = StringPrintf("Exit Status: %d ", WEXITSTATUS(status));

    if (WIFSIGNALED(status))
        reason += StringPrintf("Signaled by %d ", WTERMSIG(status));
    else if (WIFEXITED(status))
        reason += "Normally, by exit(), or return ";

    return reason;
}
