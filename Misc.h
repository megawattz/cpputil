/*
Copyright 2009 by Walt Howard
$Id: Misc.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

/**
 This file is intended as a temporary place to put stuff that doesn't deserve its own file. If enough similar stuff
 accumulates, move it to its own file, out of here.
 */

#include <Text.h>
#include <cerrno>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdarg>
#include <list>
#include <vector>
#include <functional>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <malloc.h>
#include <Spinlock.h>

// Macro for turning numbers into strings during AT COMPILE TIME.
#define Stringize(S) ReallyStringize(S)
#define STRINGIZE(S) ReallyStringize(S)
#define ReallyStringize(S) #S

#define LOCATION __FILE__ "_" STRINGIZE(__LINE__)

// These macros optimize branching. They are optimizations the gnu compiler provides for you to control the processor pipeline.
// If something is probable, the process will keep that branch in the pipeline. If it is improbable, it will keep the other branch.
// Example if (probable(item == 10)) return 22;  if (improbable(name == "joe johnson")) wrong_person++;
#ifdef __builtin_expect
#define probable(BOOL_EXPR)       __builtin_expect(not not(BOOL_EXPR), 1)
#define improbable(BOOL_EXPR)     __builtin_expect(not not(BOOL_EXPR), 0)
#else
#define probable(BOOL_EXPR)       BOOL_EXPR
#define improbable(BOOL_EXPR)     BOOL_EXPR
#endif

#define LESSER(X,Y)   (((X)<(Y))?(X):(Y))
#define GREATER(X,Y)  (((X)>(Y))?(X):(Y))

/** @brief  Construct and return an Text using a printf style format
 @note This uses a 1024 sized buffer to construct the string. If more room is needed, it reallocates (sprintf tells the caller how much more room
 is needed, it allocates it dynamically on the stack. 1024 should be adjusted higher if any significant percentage of large strings is
 being created witht his function.
 */
Text StringPrintf(int, const char *format, ...) __attribute__((format(printf, 2, 3))) __attribute__((hot));

/** @brief  Construct a string using a printf style format
 */
Text StringPrintf(const char *format, ...) __attribute__((format(printf, 1, 2))) __attribute__((hot));

/** @brief  Construct a string using a printf style format into a user supplied buffer, and return that buffer
 */
char* Sprintf(char* buffer, int size, const char *format, ...) __attribute__((format(printf, 3, 4))) __attribute__((hot));

/**
 @brief Makes an Text from an expression like this "variable << variable << variable". Supports all the operators that Textstream
 does because internally that's what it's using. This has to be a macro due to the lack of polymorphism built into the "standard" library.

 @usage example: Text test = MakeStringFromStream("hello" << ' ' << "world." << std::endl);
 @note  the std::dec is there to fix a bug. If the first item being stuck into the ostringstream is a literal string, the stringstream interprets
 it as a void* and prints a hex address instead of the string, like 0x74e8430. The std::dec putting something else first, prevents that.
 */
#define StringFromStream(STREAM_EXPRESSION)  ((static_cast<std::stringstream&>(std::ostringstream() << std::dec << STREAM_EXPRESSION)).str())

Text ScanfString(const char* source, const char* scanf_format);

/**
 @brief A strtok() analogue for c++ programmers.
 */
std::list<Text> SplitOnChars(const char* source, const char* splitters);

/** @brief   Selects one of several strings you pass in.
 *  @param   zero_based_selection  Which string to select starting with 0 for the first string.
 *  @param   string0               The first of any number of strings followed by a NULL.
 *  @return  Returns the string at the zero_based_selection index. Returns "" if not found.
 *  @note    Example: SelectString(2, "JOE", "bill", "Sam", "Martha", NULL) would return "Sam"
 *  @note    Useful to reverse an enum value back to its symbolic name.
 */
const char* SelectString(const unsigned zero_based_selection,
        const char* string0, ...);

/** @brief    Return the index of a string or -1 if the string doesn't exist in this list (case insensitive)
 *  @param    search_for_string        which string you are interested in.
 *  @param    strings_to_search_in_0   list of strings to match against the search_for_string
 *  @return                            returns the index of the string that matches or -1 if none match
 *  @note     Example: SelectString("sister", "MOTHER", "FATHER", "SISTER", "BROTHER", "UNCLE", "AUNT", NULL) would return 2
 */
int IndexOfString(const char* search_for_string,
        const char* strings_to_search_in_0, ...);

// "convert to string" function, templatized to handle many types

// Given a search_in_string and NULL terminated list of other strings, find which of those comes first in the search_in_string
Text FirstFound(const char* search_in_string, const char* search_for_strings, ...);

template<typename TYPE> Text ToString(const TYPE& stringme)
{
    std::ostringstream output;
    output << stringme;
    return output.str();
}

template<typename ITEM> inline void FromString(const Text& str, ITEM& item)
{
    std::istringstream output(str);
    output >> item;
}

template<> inline void FromString<const char*>(const Text& str, const char*& item)
{
    item = str.c_str();
}

template<> inline void FromString<Text>(const Text& str, Text& item)
{
    item = str;
}

template<typename ITEM> inline void FromString(const char*& str, ITEM& item)
{
    std::istringstream output(str);
    output >> item;
}

template<> inline void FromString<const char*>(const char*& str, const char*& item)
{
    item = str;
}

template<> inline void FromString<Text>(const char*& str, Text& item)
{
    item = str;
}

template<typename ITEM> inline ITEM FromString(const Text& str)
{
    ITEM item;
    std::istringstream output(str);
    output >> item;
    return item;
}

template<typename ITEM> inline ITEM FromString(const char*& str)
{
    ITEM item;
    std::istringstream output(str);
    output >> item;
    return item;
}

inline Text FromString(const Text& str)
{
    return str;
}

/** @brief  The name, just the filename without path, of the currently executing program.
 @note   This extracts the entire commandline of this program from /proc filesystem and returns just the first null delimited string from that.
 */
Text GetProgName();

Text GetProgPath();

Text GetProgAbsolutePath();

Text GetProgCommandLine();

Text FileNameOnly(const char* path);

Text FileNameOnly(const Text& path);

/** Wrapper for the tedious ::write(STDERR_FILENO, message.c_str(), message.size()); */
void WriteToStderr(const Text& message);

void WriteToStdout(const Text& message);

/** Replace all instances of one string, within another, without recursion problems.
 @param  target     The string within which the replacing will occur.
 @param  replacee   The string we will be replacing.
 @param  replacor   The string we will put in place when removing replacee.
 @note   A dumb replaceall might have problems if you replace "Joe" with "Joey" because
 the replaceor contains the replacee and would cause infinite recursion. This avoids
 that by starting each search for a replacee from a point AFTER then previous replacement.
 */
Text ReplaceAll(const Text& target, const Text& replacee, const Text& replaceor = "") __attribute__((hot));

template <typename VALTYPE> bool Within(const VALTYPE& low, const VALTYPE& checked, const VALTYPE& high)
{
    if (checked > high)
	return false;
    if (checked < low)
	return false;
    return true;
}

Text ExpandSpecialCharacters(const char* str);

/*
  Replaces "macros" in a string with values taken from the MAP.
*/
template <typename MAP> Text ReplaceVariables(Text target, const MAP& values, const char* delimiters = "[]")
{
    int offset(0);
    for(;;)
    {
	std::string::size_type macro = target.find(delimiters[0], offset);

	if (macro == std::string::npos)
	    break;

	std::string::size_type end = target.find(delimiters[1], macro + 1);

	if (end == std::string::npos)
	    break;

	const char* key = strndupa(target.c_str() + macro + 1, end - macro - 1);

	typename MAP::const_iterator value = values.find(key);
	if (value != values.end())
	{
	    target.replace(macro, end - macro + 1, value->second);
	    offset = macro + Text(value->second).size();
	}
	else
	{
	    offset = end + 1;
	}
    }
    return target;
}


/** Returns the full command line used to start this process (the one calling the function) */
Text ProcCmdlineToString(const Text& source);

/** HasRead
 @brief   Uses select to wait on multiple file descriptors passed in an stl vector. Waits for read ready only.
 @param   timeout_milliseconds  How long to wait in milliseconds.
 @param   fds_to_check  stl vector of ints (file descriptors) to wait on.
 @return  returns a vector of descriptors that are ready for reading.
 @note    if the vector returned is empty, no descriptors were ready. Future enhancement, probably a good idea to check for
 error ready also but didn't need that at this time.
 */
std::vector<int> HasRead(const unsigned timeout_milliseconds,
        const std::vector<int>& fds_to_check);

/** @brief Returns the demangled version of the c++ symbol */
Text DemangleCppSymbol(const char* mangled_symbol);

// Quickly create accessor member functions. If the member is named "_dataValue" the setter will be "Set_dataValue()" and the getter will be "Get_dataValue()"
#ifdef _MULTITHREAD
   #define GETTER(TYPE, MEMBER_NAME) inline TYPE get##MEMBER_NAME() { return MEMBER_NAME; }
   #define SETTER(TYPE, MEMBER_NAME) inline TYPE set##MEMBER_NAME(const TYPE& newvalue) { MEMBER_NAME = newvalue; return MEMBER_NAME; }
   #define GETSET(TYPE, MEMBER_NAME) GETTER(TYPE, MEMBER_NAME); SETTER(TYPE, MEMBER_NAME);
#else
   #define GETTER(TYPE, MEMBER_NAME) inline const TYPE& get##MEMBER_NAME() const { return MEMBER_NAME; }
   #define SETTER(TYPE, MEMBER_NAME) inline const TYPE& set##MEMBER_NAME(const TYPE& newvalue) { MEMBER_NAME = newvalue; return MEMBER_NAME; }
   #define GETSET(TYPE, MEMBER_NAME) GETTER(TYPE, MEMBER_NAME); SETTER(TYPE, MEMBER_NAME);

   #define GETTER_NONCONST(TYPE, MEMBER_NAME) inline TYPE& get##MEMBER_NAME() { return MEMBER_NAME; }
   #define SETTER_NONCONST(TYPE, MEMBER_NAME) inline TYPE& set##MEMBER_NAME(const TYPE& newvalue) { MEMBER_NAME = newvalue; return MEMBER_NAME; }
   #define GETSET_NONCONST(TYPE, MEMBER_NAME) GETTER_NONCONST(TYPE, MEMBER_NAME); SETTER_NONCONST(TYPE, MEMBER_NAME);
#endif

#define GET_STATIC(TYPE, MEMBER_NAME) inline static TYPE get##MEMBER_NAME() { return MEMBER_NAME; }
#define SET_STATIC(TYPE, MEMBER_NAME) inline static TYPE set##MEMBER_NAME(const TYPE& newvalue) { MEMBER_NAME = newvalue; return MEMBER_NAME; }
#define GETSET_STATIC(TYPE, MEMBER_NAME) GET_STATIC(TYPE, MEMBER_NAME); SET_STATIC(TYPE, MEMBER_NAME);

#define GETTER_PTR(TYPE, MEMBER_NAME) inline TYPE get##MEMBER_NAME() { return MEMBER_NAME; }
#define SETTER_PTR(TYPE, MEMBER_NAME) inline TYPE set##MEMBER_NAME(TYPE newvalue) { MEMBER_NAME = newvalue; return MEMBER_NAME; }
#define GETSET_PTR(TYPE, MEMBER_NAME) GETTER_PTR(TYPE, MEMBER_NAME); SETTER_PTR(TYPE, MEMBER_NAME);

#define strequal !strcmp

/** @brief        The following functions return a string that is the class's type. Since these are templates, there is a unique copy of the
 *                 function for each type, thus, the name needs to be calculated only once and so that name can be a static local variable.
 *   @param Type   The variable whose type needs to be derived.
 *   @return

 *   eturns an Text of the type. This will be in the same form the computer issues messages in, so for example, STL
 *                 containers and such will be long, obtuse strings.
 *   @notes        Example: Text bubba; std::cout << ClassName(bubba) << std::endl;
 */
Text ExtractClassNameFromPrettyFunction(const char* pretty_function);

/**
   Match strings from the end backward. Useful, for example, to see if one domain fits into another.
 */
const char* StrBackMatch(const char* container, const char* contained);

template<typename Type> Text ClassName(const Type& test)
{
    //constructed only once per program invocation
    static Text class_name = ExtractClassNameFromPrettyFunction(
            __PRETTY_FUNCTION__);
    return class_name;
}

Text ShortenizeClassName(const char* fullname);

Text ExtractShortClassNameFromPrettyFunction(const char* pretty_function);

template<typename Type> Text ShortClassName(const Type& test)
{
    //constructed only once per program invocation
    static Text class_name = ExtractShortClassNameFromPrettyFunction(
            __PRETTY_FUNCTION__);
    return class_name;
}

/** Like the above, but returns the name prepended with a newline and a ':' appended*/
template<typename Type> Text ClassNameLabel(const Type&)
{
    static Text class_name = ("\n" + ExtractClassNameFromPrettyFunction(
            __PRETTY_FUNCTION__) + ':');
    return class_name;
}

#define TSCASSERT(EXPRESSION) { if (not (EXPRESSION)) throw Exception(Exception::ERRNO_IGNORE, (Text(#EXPRESSION) + " failed.").c_str(), LOCATION); }

void MakeDirectory(const char* path, mode_t mode = 0777);

template<typename CLASS> void DeleteAndZero(CLASS *&ptr)
{
    delete ptr;
    ptr = 0;
}

/** return thr first non zero argument in the list. Must all be of the same type as "first" argument. */
template<typename ARG> ARG* FirstNonZero(size_t count, ARG first, ...)
{
    va_list args;
    va_start(args, first);

    if (first)
        return first;

    for (int i(1); i < count; ++i)
    {
        ARG choice = va_arg(args, ARG);
        if (choice)
            return choice;
    }

    return NULL;
}

inline double Fraction(const double& numerator, const double& denominator)
{
    return numerator / denominator;
}

inline int FractionInt(const double& numerator, const double& denominator)
{
    return int(numerator / denominator);
}

#define NO_NULL_STR(X)  ((X) ? (X) : "")

#define NO_EMPTY_STR(X) (((X) and (*X)) ? (X) : "null")

#define DELIMITER_IF_NEEDED(CSTR,DELIMITER) ((CSTR && *CSTR)?(DELIMITER):"")

#define StrdupLocal(X) strcpy(reinterpret_cast<char*>(alloca(strlen(X) + 1)), X)

#define StrcatLocal(X,Y) strcat(strcpy(reinterpret_cast<char *>(alloca(strlen(X) + strlen(Y) + 1)), X), Y)

#define StrdupLocalLimited(X,LEN) strncpy(reinterpret_cast<char*>(alloca((LEN) + 1)), (X), (LEN))

// Close to hold old fashioned malloced strings
class CStringHolder
{
    char* cstring;

public:

    CStringHolder(char* str) : cstring(str) { }

    operator char*() { return cstring; }

    ~CStringHolder()
    {
	if (cstring)
	{
	    free(cstring);
	    cstring = 0;
	}
    }
};

// stl min is a pain in the ass since it only compares two items of the same type

#define MIN(X,Y)  (((X) < (Y)) ? (X) : (Y))


inline char* Rtrim(char* str, const char* trimchars = "\t ")
{
    for(char* end = str + strlen(str); strchr(trimchars, *end) and end >= str; --end)
	*end = '\0';
    return str;
}

inline char* Ltrim(char* str, const char* trimchars = "\t ")
{
    char* start = str;
    for(/* init outside */; strchr(trimchars, *start) and *start; ++start);
    return start;
}

inline char* Trim(char* str, const char* trimchars = "\t ")
{
    return Ltrim(Rtrim(str, trimchars), trimchars);
}

char* Strlwr(char* str) __attribute__((hot));

int Strpncpy(const char* source, char* target, int length) __attribute__((hot));

char* Strtcpy(const char* source, char* target, size_t length) __attribute__((hot));

int Strpcpy(const char* source, char* target, size_t length) __attribute__((hot));

pid_t GetThreadId();

inline struct timeval GetTimeOfDay()
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv;
}

inline struct timeval* GetNewTimeOfDay()
{
    struct timeval* tv = new struct timeval;
    gettimeofday(tv, 0);
    return tv;
}

inline struct timeval TimeOfDayAdjust(const struct timeval& tm, long microseconds)
{
    // get the value in terms of seconds and microseconds
    long long ms = tm.tv_sec * 1000000LL + tm.tv_usec + microseconds;
    return {static_cast<long>(ms / 1000000L), static_cast<long>(ms % 1000000L)};
}

int FileType(const char* filename);

long long DiffTime(const struct timeval& earlier, const struct timeval& later);

long long DiffTimeSpec(const struct timespec& earlier, const struct timespec& later);

#define TimeOfDayDiff DiffTime

double DiffTimeSeconds(const struct timeval& earlier, const struct timeval& later);

Text TimeStringLocal(const char* format = "%c", time_t tm = time(0));

Text TimeStringGmt(const char* format = "%c", time_t tm = time(0));

Text StrError(int errnum);

Text ReplaceSystemVariables(const char* string);

char LastChar(const char* str);

const char* ReverseFindChar(const char* from, const char* haystack_start, char needle);

struct stat Stat(const char* filename);

bool Exists(const char* filename);

int OpenFileCount();

Text HumanTimeDiff(const time_t& earlier, const time_t& later);

char* TranslateChar(char* target, const char from, const char to);

Text Base64Decode( const char* base64string );

const char* StrWhichRev(const char* haystack, char find, int which);

Text GetCookieDomain(const char* hostname);

void SetupSignalHandlers(void (*default_handler)(int), bool core_dump);

Text GetHostname();

Text Getcwd();

void Chdir(const char* directory);

template <typename NUMBER, typename NUMBER_LOW, typename NUMBER_HIGH> NUMBER Clip(const NUMBER_LOW& low, const NUMBER& check, const NUMBER_HIGH& high)
{
    if (improbable(check < low))
	return low;

    if (improbable(check > high))
	return high;

    return check;
}

template <typename TYPE> class ScopePtr
{
    TYPE* Thing;

public:
    ScopePtr(TYPE* t = 0)
	: Thing(t)
    {
    }

    TYPE* reset(TYPE* t)
    {
	if (improbable(Thing))
	    delete Thing;
	Thing = t;
	return Thing;
    }

    TYPE* operator->()
    {
	return Thing;
    }

    ~ScopePtr()
    {
	if (Thing)
	    delete Thing;
    }
};

inline const char* isolate_type_name(const char* pretty_function)
{
    // get the pretty function, find the last = sign, everything after that is the type of the object we are interested in.
    char* type_name = strdup(strrchr(pretty_function, '=') + 2);
    type_name[strlen(type_name) - 1] = '\0';  // remove the trailing ']'
    return type_name;
}


template <typename Object> const char* GetType( const Object& object )
{
    static const char* class_name = isolate_type_name(__PRETTY_FUNCTION__);
    return class_name;
}


template <typename FROMWHAT> Text GetAsText(int (function)(void*, size_t))
{
    char buffer[2048];
    (function)(buffer, sizeof(buffer));
    return buffer;
}


template <typename FROMWHAT> Text GetAsText(int (function)(FROMWHAT, void*, size_t), FROMWHAT& what)
{
    char buffer[2048];
    (function)(what, buffer, sizeof(buffer));
    return buffer;
}

Text DecodeHttp(const char* encoded_http_request);

Text ChangeFileSuffix(const char* filename, const char* new_suffix_with_period);

Text PrefixFile(const char* filename, const char* prefix);

void SetupSignalHandlers(void (*default_handler)(int), bool core_dump);

inline char* UpperCase(char* target)
{
    for(char* c = target; *c; ++c)
	*c = toupper(*c);

    return target;
}

inline char* LowerCase(char* target)
{
    for(char* c = target; *c; ++c)
	*c = toupper(*c);

    return target;
}

class ExecAfterScope
{
    std::function<void()> Func;

public:
    ExecAfterScope(std::function<void()> func)
        : Func(func)
    {
    }

    ~ExecAfterScope()
    {
        Func();
    }
};

class PushDir
{
    Text CurrentDir;
    Text NewDir;

public:
    PushDir(const char* directory)
    : CurrentDir(Getcwd()) // save current directory
    {
        Chdir(directory);
        NewDir = directory;
    }

    PushDir(const Text& directory)
    : CurrentDir(Getcwd())
    {
        Chdir(directory);
    }

    operator const char*()
    {
        return NewDir;
    }

    ~PushDir()
    {
        Chdir(CurrentDir); // restore previous directory
    }
};

// get who owns the current executeable (the on disk image, not what user we are running as)
uid_t GetExecOwner();

// get who owns the current executeable's group (the on disk image, not what user we are running as)
uid_t GetExecGroup();

Text NumberLines(const char* text, int start = 0);

const char* stristr(const char* haystack, const char* needle);

// fast check for exact string in a set of strings. Set is |string|string2|string3|string4|.
bool InString(const char* haystack, const char* needle) __attribute__((hot));

Text FileBaseName(const char* filename = "");

Text ExitReason(int status);
