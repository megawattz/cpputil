/*
Copyright 2009 by Walt Howard
$Id: ChannelAsString.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <Stream.h>

/** @brief Simple channel class that reads and write to a Stream in text form,  separating members by \t and objects by \n
 */

class ChannelAsString: public Channel
{
    Stream& _stream;

public:
    ChannelAsString(const Channel::DIRECTION& direction, Stream& stream) :
        Channel(direction), _stream(stream)
    {
    }

    virtual Text open(const char* package_name = NULL)
    {
        if (get_direction() == Channel::OUT)
        {
            if (package_name)
                SerializeArray(*this, package_name, strlen(package_name),
                        "object");

            set_open(1);

            return package_name;
        }
        else
        {
            char buffer[128] = "";
            size_t length = SerializeArray(*this, buffer, sizeof(buffer),
                    "object");

            set_open(1);

            return buffer;
        }
    }

    virtual void close(const char* = NULL)
    {
        _stream << "\n";
    }

    template<typename PRIMITIVE> size_t serializeAny(const PRIMITIVE* object,
            const size_t count, const char* label)
    {
        size_t amount(count);
        int read_size(0);
        if (get_direction() == OUT)
        {
            _stream << label << ":";
            for (int i = 0; i < count; ++i)
                _stream << " " << *object;
            _stream << '\t';
            return count;
        }
        else
        {
            _stream.readToDelimiterString(": ");
            GenericContainer values(
                    _stream.readToDelimiterString("\t").c_str(), " "); // read entire line and create container of the values (breakup the string on whitespace)

            size_t max(0);
            for (GenericContainer::const_iterator i(values.begin()); i
                    != values.end(); ++i)
            {
                FromString(*i, const_cast<PRIMITIVE&> (object[max++]));
                if (max > count)
                    break;
            }

            incrementOffset(sizeof(PRIMITIVE) * amount);
            return max;
        }
    }

    size_t serializeAnyChar(const char* object, const size_t count,
            const char* label)
    {
        size_t amount(count);
        int read_size(0);
        if (get_direction() == OUT)
        {
            _stream << label << ": ";
            _stream << EscapeText(object, count);
            _stream << '\t';
        }
        else
        {
            _stream.readToDelimiterString(": ");
            Text text(_stream.readToDelimiterString("\t"));
            ::strncpy(const_cast<char*> (object), text.c_str(), text.size() + 1);
            UnEscapeText(const_cast<char*> (object));
        }

        incrementOffset(amount);
        return amount;
    }

    Text EscapeText(const char* const string, unsigned int length = -1) // Convert any non alphanumerics to %XX (http style escape sequences)
    {
        if (length == -1)
            length = ::strlen(string);

        char* workstring = static_cast<char*> (::alloca((length * 3) + 1));
        char* insert = workstring;

        for (const char* mark(string); *mark != '\0'; ++mark)
            if (not ::isprint(*mark) or '%' == *mark) // if *mark is a special character, escape it.
                insert += ::sprintf(insert, "%%%2.2x", (int) *mark);
            else
                *insert++ = *mark;

        *insert = '\0';

        return workstring;
    }

    char* UnEscapeText(char* string)
    {
        int offset(0);

        char* mark(string);
        for (/** declared already */; *mark; ++mark)
            if ('%' == *mark)
            {
                int val;
                ThrowOnFalse(sscanf(mark + 1, "%2.2x", &val)); // convert the next two characters from a textual hex number into an int.
                *(mark - offset) = val; // write that int back into the string as a char
                mark += 2; // advance mark an extra two positions
                offset += 2; // as we walk through "string" and fill it back in with unescaped text our insert position lags behind our parsing position
            }
            else
                *(mark - offset) = *mark;

        *(mark - offset) = '\0';
        return string;
    }

    virtual size_t serializeChar(const char* item, const size_t& count,
            const char* label)
    {
        return serializeAnyChar(item, count, label);
    }

    virtual size_t serializeUnsignedChar(const unsigned char* item,
            const size_t& count, const char* label)
    {
        return serializeAny(item, count, label);
    }

    virtual size_t serializeShort(const short int* item, const size_t& count,
            const char* label)
    {
        return serializeAny(item, count, label);
    }
    virtual size_t serializeUnsignedShort(const unsigned short int* item,
            const size_t& count, const char* label)
    {
        return serializeAny(item, count, label);
    }

    virtual size_t serializeInt(const int* item, const size_t& count,
            const char* label)
    {
        return serializeAny(item, count, label);
    }
    virtual size_t serializeUnsignedInt(const unsigned int* item,
            const size_t& count, const char* label)
    {
        return serializeAny(item, count, label);
    }

    virtual size_t serializeLong(const long* item, const size_t& count,
            const char* label)
    {
        return serializeAny(item, count, label);
    }
    virtual size_t serializeUnsignedLong(const unsigned long* item,
            const size_t& count, const char* label)
    {
        return serializeAny(item, count, label);
    }

    virtual size_t serializeLongLong(const long long* item,
            const size_t& count, const char* label)
    {
        return serializeAny(item, count, label);
    }
    virtual size_t serializeUnsignedLongLong(const unsigned long long* item,
            const size_t& count, const char* label)
    {
        return serializeAny(item, count, label);
    }

    virtual size_t serializeFloat(const float* item, const size_t& count,
            const char* label)
    {
        return serializeAny(item, count, label);
    }

    virtual size_t serializeDouble(const double* item, const size_t& count,
            const char* label)
    {
        return serializeAny(item, count, label);
    }

    virtual size_t serializeBool(const bool* item, const size_t& count,
            const char* label)
    {
        return serializeAny(item, count, label);
    }
};
