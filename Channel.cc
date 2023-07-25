/*
Copyright 2009 by Walt Howard
$Id: Channel.cc 2428 2012-08-14 15:33:13Z whoward $
*/

#include <Channel.h>

Channel::Channel(const Channel::DIRECTION& direction) :
    _direction(direction), _offset(0), _open(0)
{
}

void Channel::startOfClass(const char* classname, const char* label)
{
}

void Channel::endOfClass(const char* classname, const char* label)
{
}

Channel::~Channel()
{
}

Channel::META_TYPE Channel::MetaType(const char* class_name)
{
    Text primary = FirstFound(class_name, "Enhanced", "Text", "pair", "map", "vector", "set", "multiset", "deque", "list",
			      "multimap", "unordered_map", "unordered_multimap", "unordered_set", "list", (const char*)(0));

    if (primary.empty())
	return UNINTERESTING; // Not a recognized meta type

    if (strstr("Enhanced,Text", primary)) // transparent class (just a wrapper, don't serialize wrapper, serialize contents)
	return TRANSPARENT;

    if (strstr("vector,set,multiset,deque,list", primary))
	return SEQUENTIAL;

    if (strstr("map,unordered_map", primary))
	return ASSOCIATIVE_UNIQUE; // associative collection;

    if (strstr("multimap,unordered_multimap", primary))
	ASSOCIATIVE_MULTI; // associative collection;

    if (primary == "pair")
	return PAIR;

    return UNINTERESTING;
}
