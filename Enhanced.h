/*
   Copyright 2009 by Walt Howard
   $Id: Enhanced.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <cstring>
#include <stdarg.h>
#include <cstdlib>
#include <list>
#include <vector>
#include <limits.h>
#include <StringAsStream.h>
#include <Serialize.h>
#include <StlHelpers.h>

/* ** Template serving as an initial solution to STL container annoyances. This one allows you to construct an STL container with members.  Something
   you cannot do with the STL which prevents such things as "const" containers. It also prevents return value optimization for STL containers because
   you can only construct a default container (empty) in a return statement, i.e. "return std::vector<Text>();"

   @note For example: const Enhanced<std::vector<const char*> > MyContainer(3, "joe", "bill", "mary");

   It's also very useful to get a container from a function that returns containers, but you don't know what kind of container. Don't worry, this class
   will figure it out for you at compile time:  Enhanced<std::vector<Text> > value = getStrings();

   Since MyContainer IS-A std::vector, you can use it anywhere you'd use an std::vector.

   I'm also adding convenience functions like operator=() that lets you set any stl object to contain all the members of another stl object.
*/

template<typename UnconstMe> class UnconstMaker
{
    template<typename U> struct UnConst
    {
	typedef U UnConsted;
    };
    template<typename U> struct UnConst<const U>
    {
	typedef U UnConsted;
    };

public:
    // pass a "const U" to the UnConst template which makes the symbol U unconst (because the "const" and the "U" get separated by the template parser)
    typedef typename UnConst<const UnconstMe>::UnConsted NonConst;
};

#define Unconst(CONST_TYPE) typename UnconstMaker<CONST_TYPE>::NonConst

template<typename TO_TYPE> inline TO_TYPE Convert(const char*& from)
{
    TO_TYPE to;
    FromString(from, to);
    return to;
}

template<typename TO_TYPE> inline TO_TYPE Convert(const Text& from)
{
    TO_TYPE to;
    FromString(from, to);
    return to;
}

template<typename TO_TYPE> inline TO_TYPE Convert(const TO_TYPE& from)
{
    return from;
}


template<typename STL_CONTAINER> class Enhanced: public STL_CONTAINER
{
    typedef STL_CONTAINER Base;
    typedef typename Base::value_type ElementType;

public:

    void serialize(Channel& channel, const char* name)
    {
	Serialize(channel, dynamic_cast<STL_CONTAINER&>(*this), name);
    }

    std::string asString() const __attribute__((noinline))
    {
	return AsString(*this);
    }

    void print(bool skip) const  __attribute__((noinline))
    {
	if (probable(skip))
	    return;

	std::string temp = asString();

	std::cerr << temp;
    }

    Enhanced() {}

    inline const ElementType* atIndexPtr(int index) const
    {
	// allow negative indices meaning, "count from the back"
	if (index < 0)
	    index = this->size() + index;

	if (static_cast<unsigned>(index) >= this->size() or index < 0)
	    throw Exception(LOCATION, "Index out of range: %d", index);

	return &this->at(index);
    }

    inline ElementType* atIndexPtr(int index)
    {
	// allow negative indices meaning, "count from the back"
	if (index < 0)
	    index = this->size() + index;

	if (static_cast<unsigned>(index) >= this->size() or index < 0)
	    return 0;

	return &this->at(index);
    }

    inline const ElementType& atIndexRef(int index) const
    {
	const ElementType* it = atIndexPtr(index);
	if (not it)
	    throw Exception(LOCATION, "Index %d out of bounds", index);
	return *it;
    }

    inline ElementType& atIndexRef(int index)
    {
	ElementType* it = atIndexPtr(index);
	if (not it)
	    throw Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Index %d out of bounds", index);
	return *it;
    }

    inline void append(const ElementType& item)
    {
	Base::insert(this->end(), item);
    }

    // Because of a limitation with va_arg ONLY primitive types can be used as ElementType in this function
    template <typename ELEMENT> Enhanced(unsigned count, ELEMENT first_member, ...) // Insert a list of ElementTypes into this container initially.
    {
	va_list args;
	va_start(args, first_member);

	append(Convert<ElementType>(first_member));

	while (count-- > 1)
	{
	    ELEMENT temp = va_arg(args, ELEMENT);
	    append(Convert<ElementType>(temp));
	}

	va_end(args);
    }

    template<typename OTHER_STL_CONTAINER> Enhanced& operator+=(const OTHER_STL_CONTAINER& other)
    {
	for (typename OTHER_STL_CONTAINER::const_iterator i(other.begin()); i
		 != other.end(); ++i)
	    append(*i);
	return *this;
    }

    template<typename OTHER_STL_CONTAINER> Enhanced& operator=(const OTHER_STL_CONTAINER& other)
    {
	this->clear();
	operator+=(other);
	return *this;
    }

    template<typename OTHER_STL_CONTAINER> Enhanced(const OTHER_STL_CONTAINER& other)
    {
        operator=(other);
    }


    // like the other but splits at every single delimiter. Multiple delimiters are NOT considered one delimiter so two or more delimiters together
    // will result in empty elements which is sometimes what is wanted (like parsing a text file and preserving empty lines)
    void Init(const char* splitee, const char delimiter, const size_t max = UINT_MAX)
    {
        char* buffer = strdupa(splitee);
	char* workstring = buffer;
        size_t count(1);
	char* start = buffer;
        char* end(0);

        for (end = strchr(workstring, delimiter); end; end = strchr(end + 1, delimiter))
        {
	    *end = '\0';  // null terminate what we found.
	    append(Convert<ElementType>(start));
	    if (++count >= max)
		break;

	    start = end + 1;
	}

	// if we didn't go ALL the way, assume we want the rest of the string as a final append
	if (start and *start)
	    append(Convert<ElementType>(start));
    }


    /**
       Construct a container from a string split by a delimiter character
    */
    Enhanced(const char* splitee, const char delimiter, const size_t max = UINT_MAX)
    {
	Init(splitee, delimiter, max);
    }

    void Init(const char* splitee, const char* delimiters = " \t\r\n", const size_t max = UINT_MAX)
    {
        char* buffer = strdupa(splitee);
	char* workstring = buffer;
	char* marker;
	size_t count(1);
	char* s(0);

	for (s = strtok_r(workstring, delimiters,  &marker); s; s = strtok_r(NULL, delimiters, &marker))
	{
	    char* p = Trim(s);
	    append(Convert<ElementType>(p));
	    if (++count >= max)
		break;
	}

	// if we didn't go ALL the way, assume we want the rest of the string as a final append (s will be null if we stroke'd the entire string)
	if (s)
	{
	    int skip = strspn(marker, delimiters);
	    if (*(marker + skip)) // if we aren't at end of string
	    {
		append(Convert<ElementType>(Trim(marker + skip)));
	    }
	}
    }



    // turn a delimiter separated list of strings, into a named map where the names are in the STL_CONTAINER "names" and the values were in the splitee
    template <typename CONTAINER1, typename CONTAINER2> void ToMap(const CONTAINER1& names, const CONTAINER2& values, const size_t max = UINT_MAX)
    {
	typename CONTAINER1::const_iterator name(names.begin());
	typename CONTAINER2::const_iterator value(values.begin());

	while(name != names.end() and value != values.end())
	{
	    typedef typename UnconstMaker<typename STL_CONTAINER::value_type::first_type>::NonConst NonConstFirst;
	    append(std::make_pair(Convert<NonConstFirst>(*name), Convert<typename STL_CONTAINER::value_type::second_type>(*value)));
	}
    }

    /**
       Construct a container from a string split by delimiters using strtok_r
    */
    Enhanced(const char* splitee, const char* delimiters, const size_t max = UINT_MAX)
    {
	Init(splitee, delimiters, max);
    }


    // join to sequential containers into one associative container
    template<template<typename> class LEFT_CONTAINER, typename LEFT_ELEMENT,
	     template<typename> class RIGHT_CONTAINER, typename RIGHT_ELEMENT>
    Enhanced(const LEFT_CONTAINER<LEFT_ELEMENT>& names, const RIGHT_CONTAINER<RIGHT_ELEMENT>& values, const size_t max = UINT_MAX)
    {
	ToMap(names, values, max);
    }


    /**
       Construct a container from a string split by delimiter STRINGS (entire strings, not single characters)
    */
    Enhanced(const char* splitee, const size_t max, const char* delimiter)
    {
	char* buffer = strdupa(splitee);
	char* workstring= buffer;
	size_t count(1);
	const char* prev(workstring);
	char* next(workstring);
	int length = strlen(delimiter);

	// (if count equals or exceeds max, set the delimiter to "" which means "give me entire remainder of the string up to the terminating null")
	for (next = strstr(next, delimiter); count++ <= max and next; next = strstr(next, delimiter))
	{
	    *next = '\0';
	    append(Convert<ElementType>(Trim(prev)));
	    next += length;
	    prev = next;
	}

	if (count < max)
	    append(Convert<ElementType>(Trim(prev)));
    }

    /**
       Construct a container of pairs from a string split by delimiters using strtok_r
    */
    void Init(const char* splitee, const char* inner_delimiters, const char* outer_delimiters, const size_t max = UINT_MAX)
    {
	char* buffer = strdupa(NO_NULL_STR(splitee));
	char* workstring = buffer;

	char* outer_marker;
	size_t count(1);

	// break string up at the "outer delimiters" (like newline for example)
	for (char* block = strtok_r(workstring, outer_delimiters, &outer_marker); block and count++ <= max; block = strtok_r(NULL, outer_delimiters, &outer_marker))
	{
	    // ignore anything after a comment character
	    if (char* p = strchr(block, '#'))
		*p = '\0';

	    // find inner delimiters
	    char* first = block + strspn(block, inner_delimiters);
	    if (!*first)
		continue;

	    char* second = first + strcspn(first, inner_delimiters);

	    if (*second)
		*second++ = '\0';

	    second += strspn(second, inner_delimiters);

	    Rtrim(second, inner_delimiters);

	    typedef typename UnconstMaker<typename STL_CONTAINER::value_type::first_type>::NonConst NonConstFirst;

	    append(std::make_pair(Convert<NonConstFirst>(Trim(first)), Convert<typename STL_CONTAINER::value_type::second_type>(Trim(second))));
	}
    }

    Enhanced(const char* splitee, const char* inner_delimiters, const char* outer_delimiters, const size_t max = UINT_MAX)
    {
	Init(splitee, inner_delimiters, outer_delimiters, max);
    }
};

// Make a quick copy of a container, even if you don't know the source container type.
template<typename STL_CONTAINER> Enhanced<STL_CONTAINER> MakeEnhanced(const STL_CONTAINER& other)
{
    return Enhanced<STL_CONTAINER> (other);
}
