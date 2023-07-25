/*
Copyright 2009 by Walt Howard
$Id: ScopeStats.h 2450 2012-09-18 18:16:30Z whoward $
*/

#pragma once

#include <Singleton.h>
#include <Statistics.h>
#include <Exception.h>
#include <sys/time.h>
#include <memory.h>
#include <boost/shared_ptr.hpp>

#define SCOPESTATS(CATEGORY, STATS_OBJECT)  std::unique_ptr<ScopeStats> _stats(new ScopeStats(CATEGORY, STATS_OBJECT))

class ScopeStats
{
    static bool Enable;

    struct timespec Start;
    static timespec Resolution;

    Text Category;

    Statistics* Stats;

public:
    ScopeStats(const char* category, Statistics* stats) __attribute__((hot));

    int64_t Lap() __attribute__((hot));

    ~ScopeStats() __attribute__((hot));

    GETSET_STATIC(bool, Enable);
};

