/*
Copyright 2009 by Walt Howard
$Id: StlHelpers.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <Enhanced.h>
#include <sstream>
#include <boost/shared_ptr.hpp>

/**
   Useful stl helper functions and templates that don't exist in the actual STL
*/

/**
   OutToReturn - Convert old fashioned functions that require an output pointer to "return" a value to one that actually RETURNS the value. Useful
   for fuctions like stat, sprintf, strftime and time which have "output" parameters.

   example: struct stat_buf = OutToReturn(stat_buf, stat, "filename");

   Because there are different signatures below, the compiler will find the one that fits your function IF IT EXISTS. If the particular signature of
   your function doesn't exists below, add it.
*/

template<template<typename > class LEFT_CONTAINER, typename LEFT_ELEMENT,
	 template<typename > class RIGHT_CONTAINER, typename RIGHT_ELEMENT> LEFT_CONTAINER<
    LEFT_ELEMENT> operator+(const LEFT_CONTAINER<LEFT_ELEMENT>& left,
			    const RIGHT_CONTAINER<RIGHT_ELEMENT>& right)
{
    LEFT_CONTAINER<LEFT_ELEMENT> temp;
    temp += left;
    temp += right;
    return temp;
}


template<template<typename > class LEFT_CONTAINER, typename LEFT_ELEMENT,
	 template<typename > class RIGHT_CONTAINER, typename RIGHT_ELEMENT> LEFT_CONTAINER<
    LEFT_ELEMENT>& copy(LEFT_CONTAINER<LEFT_ELEMENT>& left,
			const RIGHT_CONTAINER<RIGHT_ELEMENT>& right)
{
    left.clear();
    left += right;
    return left;
}


// Given a single item, returns its key (useful for extracting out of associate containers into sequence containers)
template<typename KEY> const KEY& Key(const KEY& item)
{
    return item;
}

// Given a single item, returns it value (useful for extracting out of associate containers into sequence containers
template<typename VALUE> inline const VALUE& Value(const VALUE& item)
{
    return item;
}

template<typename KEY, typename VALUE> const VALUE& Value(const std::pair<KEY, VALUE>& pair)
{
    return pair.second;
}


template<typename KEY, typename VALUE> const KEY& Key(const std::pair<KEY, VALUE>& pair)
{
    return pair.first;
}

template <template <typename> class CONTAINER, typename MEMBER> std::vector<MEMBER*> GetMemberPointers(CONTAINER<MEMBER>& container)
{
    std::vector<MEMBER*> values;

    for (typename CONTAINER<MEMBER>::iterator i(container.begin()); i != container.end(); ++i)
	values.push_back(&*i);

    return values;
}


template <template <typename> class CONTAINER, typename VALUE> std::vector<VALUE> GetValues(const CONTAINER<VALUE>& container)
{
    std::vector<VALUE> values;

    for (typename CONTAINER<VALUE>::const_iterator i(container.begin()); i != container.end(); ++i)
	values.push_back(*i);

    return values;
}


template <template <typename, typename> class MAP, typename KEY, typename VALUE> std::vector<VALUE> GetValues(const MAP<KEY, VALUE>& container)
{
    std::vector<VALUE> values;

    for (typename MAP<KEY, VALUE>::const_iterator i(container.begin()); i != container.end(); ++i)
	values.push_back(i->second);

    return values;
}


template <template <typename, typename> class MAP, typename KEY, typename VALUE> std::vector<KEY> GetKeys(const MAP<KEY, VALUE>& container)
{
    std::vector<KEY> values;

    for (typename MAP<KEY, VALUE>::const_iterator i(container.begin()); i != container.end(); ++i)
	values.push_back(i->first);

    return values;
}


template<typename PREVIOUS_RETURN_TYPE, typename OUTPUT_TYPE> OUTPUT_TYPE OutToReturn(
    PREVIOUS_RETURN_TYPE( func)(OUTPUT_TYPE*))
{
    OUTPUT_TYPE output;
    (func)(&output);
    return output;
}


template<typename PREVIOUS_RETURN_TYPE, typename OUTPUT_TYPE, typename ARG> OUTPUT_TYPE OutToReturn(
    PREVIOUS_RETURN_TYPE( func)(ARG, OUTPUT_TYPE*), ARG arg)
{
    OUTPUT_TYPE output;
    (func)(arg, &output);
    return output;
}


template <template <typename, typename> class MAP, typename KEY_TYPE, typename VALUE_TYPE, typename MEMBER_TYPE> std::vector<MEMBER_TYPE> ExtractMembers(const MAP<KEY_TYPE, VALUE_TYPE>& objects, MEMBER_TYPE VALUE_TYPE::*which)
{
    std::vector<MEMBER_TYPE> members;

    for(typename MAP<KEY_TYPE, VALUE_TYPE>::const_iterator i(objects.begin()); i != objects.end(); ++i)
	members.push_back(i->second.*which);

    return members;
}


template <typename STREAM, typename ITEM> STREAM& Output(STREAM& str, const ITEM& item, const char* delimiter = "\n")
{
    str << item;
    return str;
}


template <typename STREAM, typename T1, typename T2> STREAM& Output(STREAM& str, const std::pair<T1, T2>& item, const char* delimiter = "\n")
{
    str << item.first << '=' << item.second << delimiter;
    return str;
}


/*
  @brief  Joins containers of anything having operator<< into a string.
*/
template <typename MAP> Text JoinMap(const MAP& items, const char* item_delimiter = "=", const char* record_delimiter = "; ")
{
    std::stringstream joined;

    for(typename MAP::const_iterator i(items.begin()); i != items.end(); ++i)
    {
	if (i != items.begin())
	    joined << record_delimiter;
	joined << i->first << item_delimiter << i->second;
    }

    return joined.str();
}


/*
  @brief  Joins containers of anything having operator<< into a string.
*/
template <typename MAP> Text JoinMapFirsts(const MAP& items, const char* record_delimiter = "; ")
{
    std::stringstream joined;

    for(typename MAP::const_iterator i(items.begin()); i != items.end(); ++i)
    {
	joined << i->first << record_delimiter;
    }

    return joined.str();
}


template<typename ITERATOR, typename DELIMITER> Text Join(ITERATOR start, ITERATOR end, const DELIMITER& delimiter)
{
    std::stringstream joined;

    if (start == end)
	return joined.str();

    Output(joined, *start++);

    while (start != end)
    {
        joined << delimiter;
	Output(joined, *start++);
    }

    return joined.str();
}


template<typename CONTAINER, typename DELIMITER> Text Join(const CONTAINER& container, const DELIMITER& delimiter)
{
    return Join(container.begin(), container.end(), delimiter);
}


template <template <typename> class CONTAINER, typename ELEMENT_TYPE, typename MEMBER_TYPE> std::vector<MEMBER_TYPE> ExtractMembers(const CONTAINER<ELEMENT_TYPE>& objects, MEMBER_TYPE ELEMENT_TYPE::*which)
{
    std::vector<MEMBER_TYPE> members;

    for(typename CONTAINER<ELEMENT_TYPE>::const_iterator i(objects.begin()); i != objects.end(); ++i)
	members.push_back(i->second.*which);

    return members;
}

template <template <typename, typename> class MAP, typename KEY, typename VALUE> std::map<VALUE, KEY> InverseMap(const MAP<KEY, VALUE>& stl_map)
{
    std::map<VALUE, KEY> inverse_map;
    for (typename MAP<KEY, VALUE>::const_iterator i(stl_map.begin()); i != stl_map.end(); ++i)
	inverse_map[i->second] = i->first;
    return inverse_map;
}

inline std::pair<Text, Text> SplitIntoPair(const char* str, const char* delimiters = " \t")
{
    char* first = strdupa(str);
    char* second = strpbrk(first, delimiters);

    if (probable(second))
    {
	*second++ = '\0';
	int length = strspn(second, delimiters);
	second += length;
    }
    else
    {
	// if no delimiter, point second to null terminator of first
	second = first + strlen(first);
    }

    return std::pair<Text, Text>(first, second);
}

template <typename T> boost::shared_ptr<T> MakeSharedPointer(T* item)
{
    return boost::shared_ptr<T>(item);
}
