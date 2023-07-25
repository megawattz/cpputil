/*
Copyright 2009 by Walt Howard
$Id: StringAsStream.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <ChannelReadable.h>
#include <ChannelFlatXML.h>
#include <ChannelJSON.h>
#include <Stream.h>

class StringAsStream: public Stream
{
    std::string local_data;

    std::string& _data;
    std::string::iterator _position;

    bool is_eof;

    unsigned long long _read, _written;
public:

    StringAsStream(std::string& target);

    StringAsStream(const char* data, const char* options = "");

    virtual void open(const char* resource = NULL, const char* options = NULL);

    virtual bool eof();

    virtual void close();

    virtual size_t read(const size_t max_read, char* destination);

    virtual size_t write(const size_t amount, const char* const source);

    virtual bool isWriteReady(const unsigned timeout_milliseconds = 0);

    virtual bool isReadReady(const unsigned timeout_milliseconds = 0);

    virtual const unsigned long long& get_written() const;

    virtual const unsigned long long& get_read() const;

    virtual StringAsStream* CopyNew() const;
};


template <typename TYPE> std::string AsString(const TYPE& item)
{
    std::string str;
    StringAsStream data(str);
    ChannelReadable channel(Channel::OUT, data);
    channel.open(ClassName(item).c_str());
    Serialize(channel, item, "");
    return str;
}


template <typename TYPE> std::string AsXML(const TYPE& item)
{
    std::string str;
    StringAsStream data(str);
    ChannelFlatXML channel(Channel::OUT, data);
    channel.open(ClassName(item).c_str());
    Serialize(channel, item, "");
    return str;
}


template <typename TYPE> std::string AsFlatXML(const TYPE& item)
{
    std::string str;
    StringAsStream data(str);
    ChannelFlatXML channel(Channel::OUT, data);
    channel.open(ClassName(item).c_str());
    Serialize(channel, item, "");
    return str;
}


template <typename TYPE> TYPE FromFlatXML(const Text& xml)
{
    TYPE item;
    StringAsStream data(xml);
    ChannelFlatXML channel(Channel::IN, data);
    channel.open(ClassName(item).c_str());
    Serialize(channel, item, "");
    return item;
}


template <typename TYPE> std::string AsJSON(const TYPE& item, const char* label = "item")
{
    std::string str;
    StringAsStream data(str);
    ChannelJSON channel(Channel::OUT, data);
    channel.open(ClassName(item).c_str());
    Serialize(channel, item, label);
    return str;
}


template <typename TYPE> TYPE FromJSON(const Text& json)
{
    TYPE item;
    StringAsStream data(json);
    ChannelJSON channel(Channel::IN, data);
    channel.open(ClassName(item).c_str());
    Serialize(channel, item, "");
    return item;
}


/*
  @brief  Joins containers of anything having operator<< into a string.
*/
template <typename ITERATOR, typename DELIMITER> std::string JoinAsString(ITERATOR start, ITERATOR end, const DELIMITER& delimiter)
{
    std::stringstream joined;
    joined << AsString(*start++);

    while(start != end)
	joined << delimiter << AsString(*start++);

    return joined.str();
}


template <typename CONTAINER, typename DELIMITER> std::string JoinAsString(const CONTAINER& container, const DELIMITER& delimiter)
{
    return JoinAsString(container.begin(), container.end(), delimiter);
}
