/*
   Copyright 2009 by Walt Howard
   $Id: ChannelReadable.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

/** @brief Write/Read classes as easily readable text
 */

#include <Channel.h>
#include <Stream.h>

class ChannelReadable: public Channel
{
    Stream& _stream;
    unsigned _indent;

    Text indent(const int amount)
    {
        return Text(
	    "                                                                                        ",
	    amount * 4);
    }

public:
    ChannelReadable(const Channel::DIRECTION& direction, Stream& stream) :
        Channel(direction), _stream(stream), _indent(0)
    {
    }

    virtual Text open(const char* package_name = NULL)
    {
        if (get_direction() == Channel::OUT)
        {
            set_open(1);
            return package_name;
        }

	throw(Exception(LOCATION, "ChannelReadable only serializes OUT"));

	return "";
    }

    virtual void startOfClass(const char* classname, const char* label)
    {
        if (get_direction() == Channel::OUT)
        {
            _stream << indent(_indent);
            _stream << label << " (" << ShortenizeClassName(classname) << "):\n";
            _stream << indent(_indent) << "{\n";
            _indent++;
        }
        else
        {
            throw(Exception(LOCATION, "ChannelReadable only serializes OUT"));
        }

        incrementOffset(_stream.gcount());
    }

    virtual void endOfClass(const char*, const char*)
    {
        if (get_direction() == Channel::OUT)
        {
            --_indent;
            _stream << indent(_indent) << "}\n";
        }
        else
        {
            throw(Exception(LOCATION, "ChannelReadable only serializes OUT"));
        }
        incrementOffset(_stream.gcount());
    }

    template<typename PRIMITIVE> size_t serializeAny(PRIMITIVE* object, const size_t count, const char* label)
    {
        size_t amount(count);

        if (get_direction() == OUT)
        {

            _stream << indent(_indent);
            _stream << label << ": ";

            for (unsigned i(0); i < count; ++i)
            {
                _stream << object[i] << ' ';
            }

            _stream << '\n';

            incrementOffset(_stream.gcount());
        }
        else
            throw Exception(LOCATION, "ChannelReadable only serializes OUT");

        return amount;
    }

    size_t serializeAnyChar(char* object, const size_t count, const char* label)
    {
        size_t number_of_elements(0);

        if (get_direction() == OUT)
        {
            _stream << indent(_indent);
            _stream << label << ": " << object << '\n';
        }
        else
        {
            throw Exception(LOCATION, "ChannelReadable only serializes OUT");
        }
        incrementOffset(_stream.gcount());
        return number_of_elements;
    }

    virtual size_t serializeChar(char* item, const size_t& count, const char* label)
    {
        return serializeAnyChar(item, count, label);
    }

    virtual size_t serializeUnsignedChar(unsigned char* item, const size_t& count, const char* label)
    {
        return serializeAny(item, count, label);
    }

    virtual size_t serializeShort(short int* item, const size_t& count, const char* label)
    {
        return serializeAny(item, count, label);
    }

    virtual size_t serializeUnsignedShort(unsigned short int* item, const size_t& count, const char* label)
    {
        return serializeAny(item, count, label);
    }

    virtual size_t serializeInt(int* item, const size_t& count, const char* label)
    {
        return serializeAny(item, count, label);
    }

    virtual size_t serializeUnsignedInt(unsigned int* item, const size_t& count, const char* label)
    {
        return serializeAny(item, count, label);
    }

    virtual size_t serializeLong(long* item, const size_t& count, const char* label)
    {
        return serializeAny(item, count, label);
    }

    virtual size_t serializeUnsignedLong(unsigned long* item, const size_t& count, const char* label)
    {
        return serializeAny(item, count, label);
    }

    virtual size_t serializeLongLong(long long* item, const size_t& count, const char* label)
    {
        return serializeAny(item, count, label);
    }

    virtual size_t serializeUnsignedLongLong(unsigned long long* item, const size_t& count, const char* label)
    {
        return serializeAny(item, count, label);
    }

    virtual size_t serializeFloat(float* item, const size_t& count, const char* label)
    {
        return serializeAny(item, count, label);
    }

    virtual size_t serializeDouble(double* item, const size_t& count, const char* label)
    {
        return serializeAny(item, count, label);
    }

    virtual size_t serializeBool(bool* item, const size_t& count, const char* label)
    {
        return serializeAny(item, count, label);
    }
};
