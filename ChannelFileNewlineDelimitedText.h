/*
   Copyright 2009 by Walt Howard
   $Id: ChannelFileNewlineDelimitedText.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <Channel.h>
#include <DebugJunk.h>

class ChannelFileNewlineDelimitedText: public Channel
{
    int _file_descriptor;
    char _read_buffer[32]; // NOT trying for speed optimization. Needed to do "read up to newline" on a simple file descriptor.
    char* _insert_point;
    char* _read_point;
    char* const _end; // Never change the end pointer

public:
    ChannelFileNewlineDelimitedText(const Channel::DIRECTION& direction,
				    int file_descriptor) :
        Channel(direction), _file_descriptor(file_descriptor), _insert_point(
	    _read_buffer), _read_point(_end), _end(_read_buffer
						   + sizeof(_read_buffer) - 1)
    {
        THROW_ON_ERROR(_file_descriptor);
        *_end = '\0';
        *_read_buffer = '\0';
    }

    Text ReadToDelimiter(char delimiter)
    {
        char* end_of_next = ::strchr(_read_point, delimiter); // find the delimiter

        while (not end_of_next) // if there is no newline in the buffer, read in new stuff
        {
            // move any remaining junk to the beginning of the buffer in prepartion for refilling the buffer
            int length(_end - _read_point);
            ::memmove(_read_buffer, _read_point, length);
            _read_point = _read_buffer;
            _insert_point = _read_point + length;
            if (_insert_point == _end)
                ThrowSerializationException(*this,
					    "_read_buffer not big enough in Serialization IN");
            _insert_point = _read_buffer + length;
            int amount_read = ::read(_file_descriptor, _insert_point, _end
				     - _insert_point); // try to completely fill the buffer;
            if (-1 == amount_read)// and errno != EAGAIN)
                ThrowSerializationException(*this, "Serializing IN: ", -1);
            if (0 == amount_read)
                ThrowSerializationException(*this, "Unexpected end of stream",
					    -1);
            _insert_point += amount_read;
            *_insert_point = '\0'; // just in case NOT an entire buffer was read, 0 terminate and use what we got

            end_of_next = ::strchr(_read_point, delimiter);
        }

        char* rval_start = _read_point;
        _read_point = end_of_next + 1; // skip past the delimiter
        incrementOffset(_read_point - rval_start);
        return Text(rval_start, _read_point - rval_start - 1);
    }

    Text EscapeText(const char* string) // Convert any non alphanumerics to %XX (http style escape sequences)
    {
        char* workstring =
	    static_cast<char*> (::alloca((::strlen(string) * 3) + 1));
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
                THROW_ON_FALSE(sscanf(mark + 1, "%2x", &val)); // convert the next two characters from a textual hex number into an int.
                *(mark - offset) = val; // write that int back into the string as a char
                mark += 2; // advance mark an extra two positions
                offset += 2; // as we walk through "string" and fill it back in with unescaped text our insert position lags behind our parsing position
            }
            else
                *(mark - offset) = *mark;

        *(mark - offset) = '\0';
    }

    template<typename PRIMITIVE> size_t serializeAny(
	const PRIMITIVE* const_object, const size_t& count,
	const char* name)
    {
        PRIMITIVE* object = const_cast<PRIMITIVE*> (const_object);
        if (get_direction() == OUT) // Write out the elements one by one, only putting a \t at the end of the grouping (probably a string)
        {
            int total_written(0);
            for (size_t i(0); i < count; ++i)
            {
                Text escaped(EscapeText(ToString(*(object++)).c_str())
			     += '\t');
                int written = ::write(_file_descriptor, escaped.c_str(),
				      escaped.size());
                //debugout << name << ':' << escaped << '\n';
                THROW_ON_ERROR(written);
                total_written += written;
            }
            incrementOffset(total_written);
            return count;
        }
        else
        {
            for (size_t i(0); i < count; ++i)
            {
                Text unescaped(ReadToDelimiter('\t')); // read the next item
                *object = FromString<PRIMITIVE> (unescaped.c_str());
                //debugout << name << ':' << unescaped << '=' << *object << '\n';
                ++object;
            }
            return count;
        }
    }

    size_t serializeCharArray(const char* const_object, const size_t& count,
			      const char* name)
    {
        char* object = const_cast<char*> (const_object);
        if (get_direction() == OUT) // Write out the elements one by one, only putting a \t at the end of the grouping (probably a string)
        {
            Text escaped(EscapeText(object));
            int written_line = ::write(_file_descriptor, escaped.c_str(),
				       escaped.size());
            THROW_ON_ERROR(written_line);
            int written_tab = ::write(_file_descriptor, "\t", 1);
            THROW_ON_ERROR(written_tab);
            //debugout << name << ':' << object << '\n';
            incrementOffset(written_line + written_tab);
            return count;
        }
        else
        {
            Text unescaped(ReadToDelimiter('\t')); // read the entire grouping.
            strncpy(object, unescaped.c_str(), count);
            //debugout << name << ':' << unescaped << '\n';
            return std::min(count, unescaped.length());
        }
    }

    virtual void EndOfClass(const char*) // write or read a newline character
    {
        if (get_direction() == Channel::OUT)
            ::write(_file_descriptor, "\n", 1);
        else
            ReadToDelimiter('\n');
    }

    virtual size_t serializeChar(const char* item, const size_t& count,
				 const char* name)
    {
        return serializeCharArray(item, count, name);
    }

    virtual size_t serializeUnsignedChar(const unsigned char* item,
					 const size_t& count, const char* name)
    {
        return serializeAny(item, count, name);
    }

    virtual size_t serializeShort(const short int* item, const size_t& count,
				  const char* name)
    {
        return serializeAny(item, count, name);
    }
    virtual size_t serializeUnsignedShort(const unsigned short int* item,
					  const size_t& count, const char* name)
    {
        return serializeAny(item, count, name);
    }

    virtual size_t serializeInt(const int* item, const size_t& count,
				const char* name)
    {
        return serializeAny(item, count, name);
    }
    virtual size_t serializeUnsignedInt(const unsigned int* item,
					const size_t& count, const char* name)
    {
        return serializeAny(item, count, name);
    }

    virtual size_t serializeLong(const long* item, const size_t& count,
				 const char* name)
    {
        return serializeAny(item, count, name);
    }
    virtual size_t serializeUnsignedLong(const unsigned long* item,
					 const size_t& count, const char* name)
    {
        return serializeAny(item, count, name);
    }

    virtual size_t serializeLongLong(const long long* item,
				     const size_t& count, const char* name)
    {
        return serializeAny(item, count, name);
    }
    virtual size_t serializeUnsignedLongLong(const unsigned long long* item,
					     const size_t& count, const char* name)
    {
        return serializeAny(item, count, name);
    }

    virtual size_t serializeFloat(const float* item, const size_t& count,
				  const char* name)
    {
        return serializeAny(item, count, name);
    }

    virtual size_t serializeDouble(const double* item, const size_t& count,
				   const char* name)
    {
        return serializeAny(item, count, name);
    }

    virtual size_t serializeBool(const bool* item, const size_t& count,
				 const char* name)
    {
        return serializeAny(item, count, name);
    }

};
