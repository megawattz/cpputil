#pragma once

#include <Exception.h>
#include <Misc.h>
#include <sys/time.h>
#include <unordered_map>
#include <map>

class Tally
{
    int64_t Events;
    int64_t Total;
    struct timeval Start;

    double Lowest;
    double Highest;

public:
    Tally();

    void operator+=(unsigned value) __attribute__((hot));
    void Set(unsigned value);
    void Zero();
    double Average() const;
    long AverageInt() const;
    double PerSecond(struct timeval* asof) const;
    Tally& operator+=(const Tally& tally);

    GETSET(int64_t, Events);
    GETSET(int64_t, Total);
    GETSET(struct timeval, Start);
    GETSET(double, Lowest);
    GETSET(double, Highest);
};

class TextHasher
{
public:
    size_t operator()(const Text& t) const
    {
        //  http://stackoverflow.com/questions/2571683/djb2-hash-function
        //  (Adapted for a string object which allows access to naked c string pointer)
        size_t hash = 5381;
        for(const char* p = t.c_str(); *p; p++)
            hash = ((hash << 5) + hash ) + *p;
        return hash;
    }
};

//typedef Enhanced<std::map<Text, SHAREDSTAT>> SHAREDSTATMAP;
//otypedef Enhanced<std::unordered_map<Text, SHAREDSTAT, TextHasher>> SHAREDSTATMAP;

class Statistics
{
    static volatile bool Enabled;

    //typedef std::map<Text, Tally> PERFORMANCE;
    typedef std::unordered_map<Text, Tally, TextHasher> PERFORMANCE;
    PERFORMANCE Performances;
    struct timeval LastClear;
    struct timeval Now;

    int Multiplier; // if we are sampling 1/X events, multiply counts by X

public:
    GETSET(struct timeval, LastClear);
    GETSET(struct timeval, Now);
    GETSET(int, Multiplier);
    GETSET(PERFORMANCE, Performances);
    inline int getStatCount() { return Performances.size(); }
    static void setEnabled(volatile bool value);
    static bool getEnabled() __attribute__((hot));
    Statistics();
    void Freeze();
    void Zero(const char* value = 0);
    void Clear(const char* value = 0);
    void Add(const char* category, int data);
    void Add(const Text& category, int data);
    void Set(const char* category, int data);
    void Set(const Text& category, int data);
    Text AsString() const;
    Text Meta(const char* field_separator = ": ", const char* record_separator = "\n") const;
    Text StaticMeta(const char* field_separator = ": ", const char* record_separator = "\n") const;
    Text AsStringSorted();
    Text AsRawString() const;
    Text AsNameValuePairs(const char* field_separator = ": ", const char* record_separator = "\n") const;
    Text AsNameValuePairsSegment(const char* segment = NULL, const char* field_separator = ": ", const char* record_separator = "\n") const;
    Text AsNameValuePairsTotal(const char* segment = NULL, const char* field_separator = ": ", const char* record_separator = "\n") const;
    Text AsJSON(const char* segment = NULL) const;
    Text CountsAsJSON() const;
    Text AsJSONTotal(const char* segment = NULL) const;
    std::vector<Text> AsFormatted(const char* printf_format_count = "%s:%d|c\n", const char* printf_format_gauge = "%s:%d|g\n", const size_t max_size = 9999999) const;
    Statistics& Merge(const Statistics& other); // merge in another stat object by adding values
    Statistics& operator+=(const Statistics& other); // merge in another stat object by adding values
};
