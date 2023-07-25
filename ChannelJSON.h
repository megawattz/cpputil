/*
Copyright 2009 by Walt Howard
$Id: ChannelJSON.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

/** @brief Write/Read classes as XML text
 */

#include <Channel.h>
#include <Stream.h>

class ChannelJSON: public Channel
{
    Stream& _stream;
    unsigned _indent;
    int Verbosity;
    int NewLines;
    std::vector<int> Scope;

    Text indent(const int amount)
    {
        return Text("                                                                                        ", amount * 4);
    }

public:
    ChannelJSON(const Channel::DIRECTION& direction, Stream& stream, int verbosity = 0) :
	Channel(direction), _stream(stream), _indent(0), Verbosity(verbosity), NewLines(1)
    {
    }

    virtual Text open(const char* package_name = NULL)
    {
        if (get_direction() == Channel::OUT)
        {
            if (not package_name)
                throw Exception(LOCATION, "You must provide a package name when serializing OUT");
            set_open(1);
            _stream << "{ package_type: " << package_name << " }\n";
            return package_name;
        }
        else
        {
        }
	return package_name;
    }


    virtual void startOfClass(const char* classname, const char* label)
    {
        if (get_direction() == Channel::OUT)
        {
	    Channel::META_TYPE meta_type = MetaType(classname);
	    switch(meta_type)
	    {
	    case UNINTERESTING: // object (C++ primitives don't come into this function)
	    default:
		_stream << indent(_indent);
		_stream << label
			<< (Verbosity ? StringPrintf(0, "(%s)", classname) : "") << ":"
			<< (NewLines ? "\n" + indent(_indent) : "") << "{\n";
		_indent++;
		break;

	    case ASSOCIATIVE_MULTI:
	    case ASSOCIATIVE_UNIQUE:
	    case SEQUENTIAL:
		_stream << indent(_indent);
		_stream << label << (Verbosity ? StringPrintf(0, "(%s)", classname) : "")  << ":\n" << indent(_indent) << "[\n";
		_indent++;
	    }
        }
        else
        {
        }

        incrementOffset(_stream.gcount());
    }

    virtual void endOfClass(const char* classname, const char* label = NULL)
    {
        if (get_direction() == Channel::OUT)
	{
	    switch(MetaType(classname))
	    {
	    case UNINTERESTING:
	    default:
		_stream << "\n" << indent(--_indent) << "}\n";
		break;

	    case ASSOCIATIVE_MULTI:
	    case ASSOCIATIVE_UNIQUE:
	    case SEQUENTIAL:
		_stream << "\n" << indent(--_indent) << "]\n";
	    }
        }
        else
        {
        }
        incrementOffset(_stream.gcount());
    }

    template<typename PRIMITIVE> size_t serializeAny(PRIMITIVE* object, const size_t count, const char* label)
    {
        size_t amount(count);

        if (get_direction() == OUT)
        {
	    if (strequal(label,"count"))
	    {
		return 0;
	    }

            if (count > 1)
            {
		_stream << indent(_indent) << label << ": [\n";
            }
	    else
	    {
		_stream << indent(_indent) << label << ": ";
	    }

            for (unsigned i(0); i < count; ++i)
            {
                _stream << object[i] << (Verbosity ? StringPrintf(0, "(%s)", ClassName(object[0]).c_str()) : "") << ",\n";
            }

	    if (count > 1)
		_stream << " ],\n";

	    incrementOffset(_stream.gcount());
        }
        else
        {
        }

        return amount;
    }

    size_t serializeAnyChar(char* object, const size_t& count, const char* label)
    {
	_stream << indent(_indent);
	_stream << label << ": \"" << object  << (Verbosity ? StringPrintf(0, "(%s)", ClassName(object).c_str()) : "") << "\",\n";
	return _stream.gcount();
    }

    virtual size_t serializeChar(char* item, const size_t& count, const char* label)
    {
        return serializeAnyChar(item, count, label);
    }

    virtual size_t serializeUnsignedChar(unsigned char* item, const size_t& count, const char* label)
    {
        return serializeAnyChar((char*)item, count, label);
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
