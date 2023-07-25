/*
Copyright 2009 by Walt Howard
$Id: ChannelFile.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

/** @brief Simple channel class that reads and write to a file handle. It serializes the raw data not translating it into strings, or xml or anything,
 just the raw internal representation.
 */

class ChannelFile: public Channel
{
    int _file_descriptor;

public:
    ChannelFile(const Channel::DIRECTION& direction, int file_descriptor) :
        Channel(direction), _file_descriptor(file_descriptor)
    {
        THROW_ON_ERROR(_file_descriptor);
    }

    template<typename PRIMITIVE> size_t serializeAny(const PRIMITIVE* object, const size_t count)
    {
        size_t amount(count);

        int bytes(0);
        int rval;
        if (get_direction() == OUT)
        {
            rval = ::write(_file_descriptor, &amount, sizeof(amount));
            THROW_ON_ERROR(rval);
            bytes += rval;
            rval = ::write(_file_descriptor, object, sizeof(PRIMITIVE) * amount);
            THROW_ON_ERROR(rval);
            bytes += rval;
        }
        else
        {
            rval = ::read(_file_descriptor, &amount, sizeof(amount));
            THROW_ON_ERROR(rval);
            bytes += rval;
            rval = ::read(_file_descriptor, const_cast<PRIMITIVE*> (object),
                    sizeof(PRIMITIVE) * amount);
            THROW_ON_ERROR(rval);
            bytes += rval;
        }
	incrementOffset(bytes);
        return amount;
    }

    virtual size_t serializeChar(const char* item, const size_t& count,
            const char* name)
    {
        return serializeAny(item, count);
    }
    virtual size_t serializeUnsignedChar(const unsigned char* item,
            const size_t& count, const char* name)
    {
        return serializeAny(item, count);
    }

    virtual size_t serializeShort(const short int* item, const size_t& count,
            const char* name)
    {
        return serializeAny(item, count);
    }
    virtual size_t serializeUnsignedShort(const unsigned short int* item,
            const size_t& count, const char* name)
    {
        return serializeAny(item, count);
    }

    virtual size_t serializeInt(const int* item, const size_t& count,
            const char* name)
    {
        return serializeAny(item, count);
    }
    virtual size_t serializeUnsignedInt(const unsigned int* item,
            const size_t& count, const char* name)
    {
        return serializeAny(item, count);
    }

    virtual size_t serializeLong(const long* item, const size_t& count,
            const char* name)
    {
        return serializeAny(item, count);
    }
    virtual size_t serializeUnsignedLong(const unsigned long* item,
            const size_t& count, const char* name)
    {
        return serializeAny(item, count);
    }

    virtual size_t serializeLongLong(const long long* item,
            const size_t& count, const char* name)
    {
        return serializeAny(item, count);
    }
    virtual size_t serializeUnsignedLongLong(const unsigned long long* item,
            const size_t& count, const char* name)
    {
        return serializeAny(item, count);
    }

    virtual size_t serializeFloat(const float* item, const size_t& count,
            const char* name)
    {
        return serializeAny(item, count);
    }

    virtual size_t serializeDouble(const double* item, const size_t& count,
            const char* name)
    {
        return serializeAny(item, count);
    }

    virtual size_t serializeBool(const bool* item, const size_t& count,
            const char* name)
    {
        return serializeAny(item, count);
    }
};
