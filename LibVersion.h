#include <Misc.h>

#pragma once

class LibVersion
{
 public:
    static const char* getMajor() { return STRINGIZE(version: ); }
    static const char* getRevision() { return STRINGIZE(revision: 39924b2); }
    static const char* getBuild() { return STRINGIZE(revision: 39924b2); }
    static const char* getFullInfo() { return STRINGIZE(version_info:  39924b2 master 07/24/23-23:32:33);}
};

#define LibVersionString LibVersion::getFullInfo()
#define LibRevision LibVersion::getRevision()
