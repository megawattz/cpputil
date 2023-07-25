/*
Copyright 2009 by Walt Howard
$Id: ScopeStats.cc 2451 2012-09-18 21:00:48Z whoward $
*/

#include <ScopeStats.h>

bool ScopeStats::Enable = 0;

timespec ScopeStats::Resolution = {0, 0};

ScopeStats::ScopeStats(const char* category, Statistics* stats)
    : Category(category), Stats(stats)
{
    if (not Enable)
        return;

    if (not Resolution.tv_nsec)
        THROW_ON_ERROR(::clock_getres(CLOCK_MONOTONIC, &ScopeStats::Resolution));

    THROW_ON_ERROR(clock_gettime(CLOCK_MONOTONIC, &Start));
}

// needed if you want to print the time as the destructor just sends stats.
int64_t ScopeStats::Lap()
{
    if (not Enable)
        return 0;

    struct timeval now;

    timespec end;
    THROW_ON_ERROR(clock_gettime(CLOCK_MONOTONIC, &end));

    int64_t diff = DiffTimeSpec(Start, end)/Resolution.tv_nsec/1000;  // convert nanoseconds to microseconds

    return diff;
}

ScopeStats::~ScopeStats()
{
    if (not Enable)
        return;
    struct timeval now;

    timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);

    int64_t diff = DiffTimeSpec(Start, end)/Resolution.tv_nsec/1000;  // convert nanoseconds to microseconds

    Stats->Add(Category, diff);
}
