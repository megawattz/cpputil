#pragma once

#include <Misc.h>
#include <Exception.h>
#include <Text.h>
#include <ostream>
#include <boost/shared_ptr.hpp>
#include <pcre.h>

struct PcreHolder
{
    pcre* CompiledPattern;

    PcreHolder(pcre* compiled_pattern)
	: CompiledPattern(compiled_pattern)
    {
    }

    operator pcre*()
    {
	return CompiledPattern;
    }

    ~PcreHolder()
    {
	if (CompiledPattern)
	    pcre_free(CompiledPattern);
    }
};

class Regex: public Text
{
    bool FakeRegex;  // if the searched string isn't really a regex, consider it a strstr on the beginning of the string
    boost::shared_ptr<PcreHolder> CompiledPattern;
    int Options;

    static bool HasRegex(const char* str);

public:
    Regex(const char* expression = "", const int options = 0);

    bool Search(const char* haystack) const __attribute__((hot));
};

inline std::ostream& operator<<(std::ostream& output, const Regex& regex)
{
    output << regex.c_str();
    return output;
}

template<> inline void FromString<Regex>(const Text& str, Regex& item)
{
    item = Regex(str);
}
