#include <Regex.h>

bool Regex::HasRegex(const char* str)
{
    for (const char* p = NO_NULL_STR(str); *p; ++p)
	if (strchr("[]{}?.|$^*|+", *p))
	    if (*(p - 1) != '\\')
		return true;

    return false;
}

Regex::Regex(const char* expression, const int options)
    : Text(NO_NULL_STR(expression)), FakeRegex(false), Options(options)
{
    if (not HasRegex(NO_NULL_STR(expression)))
    {
	FakeRegex = true;
	return ;
    }

    const char *error;
    int error_offset;

    pcre* temp = pcre_compile(c_str(), Options, &error, &error_offset, 0);

    if (not temp)
	throw Exception(LOCATION, "Error %s compiling regular expression %s at %s", error, expression, expression + error_offset);

    CompiledPattern.reset(new PcreHolder(temp));
}


bool Regex::Search(const char* haystack) const
{
    if (probable(FakeRegex))
	return strstr(haystack, Text::c_str());

    int rval = pcre_exec(*CompiledPattern.get(), 0, haystack, strlen(haystack), 0, 0, 0, 0);
    if (not rval)
	return true;

    if (rval == -1)
	return false;

    throw Exception(LOCATION, "Error %d while executing regex: %s", rval, c_str());
}
