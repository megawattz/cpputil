/*
Copyright 2009 by Walt Howard
$Id: Text.h 2449 2012-09-18 17:57:20Z whoward $
*/

#pragma once

#include <cstring>
#include <string>

class Channel;
class Regex;

/*
std::string terribly deficient. This is just a wrapper around it that adds some functionality that I personally want 
and as a placeholder for future expansion, for example, to handle UTF-8 strings if the standard library never gets 
around to doing that.
*/

class Text: public std::string
{
public:

    static const Text EmptyString;
    static const Text Empty;

    Text(const char* str = "")  __attribute__((hot));

    Text(const char* str, int length) __attribute__((hot));

    Text(const std::string& root) __attribute__((hot));

    operator const char*() const __attribute__((hot));

    operator bool() const __attribute__((hot));

    // case insensitive ==
    bool operator*=(const char* other)  const __attribute__((hot));

    // compare to regex (not anchored), i.e, "contains" regex?
    bool operator%=(const Regex& other)  const __attribute__((hot));

    void serialize(Channel& channel, const char* label = NULL);

    template <typename OTHER> Text& operator<<(const OTHER& other)
    {
	append(ToString(other));
	return *this;
    }

    ~Text()  __attribute__((hot));
};

inline bool operator==(const Text& left, const char* right)
{
    return not ::strcmp(left.c_str(), right);
}

inline bool operator==(const char* left, const Text& right)
{
    return not ::strcmp(left, right.c_str());
}

inline bool operator==(const Text& left, const Text& right)
{
    return not ::strcmp(left.c_str(), right.c_str());
}
