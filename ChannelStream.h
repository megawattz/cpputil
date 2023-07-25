/*
Copyright 2009 by Walt Howard
$Id: ChannelStream.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <Stream.h>

/** @brief Simple channel class that reads and write to a Stream. It serializes the raw data not translating it into strings, or xml or anything, just
 the raw internal representation.
 */

class ChannelStream: public Channel
{
    Stream& _stream;

public:
    ChannelStream(const Channel::DIRECTION& direction, Stream& stream) :
        Channel(direction), _stream(stream)
    {

    }

    virtual Text open(const char* package_name = NULL)
    {
        if (get_direction() == Channel::OUT)
        {
            if (not package_name)
                throw Exception(LOCATION, "package_name required when serializing OUT");
	    Text name(package_name);
            set_open(1);
            Serialize(*this, name, "package_header");
            return name;
        }
        else
        {
	    Text name;
            set_open(1);
            Serialize(*this, name, "package_header");
            return name;
        }
    }

    template<typename PRIMITIVE> size_t serializeAny(PRIMITIVE* object, const size_t count)
    {
        size_t amount(count);
        //int read_size(0);
        if (get_direction() == OUT)
        {
            _stream.writeAll(sizeof(amount),
                    reinterpret_cast<const char*> (&amount));
            _stream.writeAll(sizeof(PRIMITIVE) * amount,
                    reinterpret_cast<const char*> (object));
        }
        else
        {
            while (not _stream.readAll(sizeof(amount),
                    const_cast<char*> (reinterpret_cast<const char*> (&amount))))
                sleep(1);
            while (not _stream.readAll(sizeof(PRIMITIVE) * amount,
                    const_cast<char*> (reinterpret_cast<const char*> (object))))
                sleep(1);
        }

        incrementOffset(sizeof(PRIMITIVE) * amount);
        return amount;
    }

    virtual size_t serializeChar(char* item, const size_t& count, const char* name)
    {
        return serializeAny(item, count);
    }
    virtual size_t serializeUnsignedChar(unsigned char* item, const size_t& count, const char* name)
    {
        return serializeAny(item, count);
    }

    virtual size_t serializeShort(short int* item, const size_t& count, const char* name)
    {
        return serializeAny(item, count);
    }
    virtual size_t serializeUnsignedShort(unsigned short int* item, const size_t& count, const char* name)
    {
        return serializeAny(item, count);
    }

    virtual size_t serializeInt(int* item, const size_t& count, const char* name)
    {
        return serializeAny(item, count);
    }

    virtual size_t serializeUnsignedInt(unsigned int* item, const size_t& count, const char* name)
    {
        return serializeAny(item, count);
    }

    virtual size_t serializeLong(long* item, const size_t& count, const char* name)
    {
        return serializeAny(item, count);
    }

    virtual size_t serializeUnsignedLong(unsigned long* item, const size_t& count, const char* name)
    {
        return serializeAny(item, count);
    }

    virtual size_t serializeLongLong(long long* item, const size_t& count, const char* name)
    {
        return serializeAny(item, count);
    }

    virtual size_t serializeUnsignedLongLong(unsigned long long* item, const size_t& count, const char* name)
    {
        return serializeAny(item, count);
    }

    virtual size_t serializeFloat(float* item, const size_t& count, const char* name)
    {
        return serializeAny(item, count);
    }

    virtual size_t serializeDouble(double* item, const size_t& count, const char* name)
    {
        return serializeAny(item, count);
    }

    virtual size_t serializeBool(bool* item, const size_t& count, const char* name)
    {
        return serializeAny(item, count);
    }
};
