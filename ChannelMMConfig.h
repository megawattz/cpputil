/*
Copyright 2009 by Walt Howard
$Id: ChannelMMConfig.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

/** @brief Write/Read classes as XML text
 */

#include <Channel.h>
#include <Stream.h>

class ChannelMMConfig: public Channel
{
    FileDescriptorStream& _stream;
    unsigned _indent;

    Text indent(const int amount)
    {
        return Text(
                "                                                                                        ",
                amount * 4);
    }

public:
    ChannelMMConfig(const Channel::DIRECTION& direction,
            FileDescriptorStream& stream) :
        Channel(direction), _stream(stream), _indent(0)
    {
    }

    virtual Text open(const char* package_name = NULL)
    {
        if (get_direction() == Channel::OUT)
        {
            if (not package_name)
                THROW(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "You must provide a package name when serializing OUT"));
            _stream << "<package type=\"" << package_name << "\" />\n";
            set_open(1);
            return package_name;
        }
        else
        {
            Text package_line(_stream.readToDelimiterStringWithTimeout(
                    "\n", Stream::INCLUDE_DELIMITER, 5000));
            set_open(1);
            return ExtractXMLAttributeNameValue(package_line, "type");
        }
    }

    Text ExtractXMLAttributeNameValue(const Text& xml,
            const char* tag)
    {
        const char* tagstart = strstr(xml.c_str(), tag);

        if (not tagstart)
            return "";

        char value[128] = "";
        // read and throw away everything that is not an equal sign. Then throw away anything that IS an equal sign, whitespace or a quote. Then read the value which is everything up to the next quote.
        int rval = sscanf(tagstart, "%*[^=]%*[= \t\"]%127[^\"]", value);
        if (rval != 1)
            THROW(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Unable to get value of tag"));

        return value;
    }

    virtual void startOfClass(const char* classname, const char* label)
    {
        if (get_direction() == Channel::OUT)
        {
            _stream << indent(_indent++);
            _stream << "<class type=\"" << ShortenizeClassName(classname) << "\" label=\"" << label
                    << "\">\n";
        }
        else
        {
            Text junk = _stream.readToDelimiterStringWithTimeout(">",
                    Stream::INCLUDE_DELIMITER, 5000); // throw away
        }

        incrementOffset(_stream.gcount());
    }

    virtual void endOfClass(const char*, const char*)
    {
        if (get_direction() == Channel::OUT)
        {
            _stream << indent(--_indent);
            _stream << "</class>\n";
        }
        else
        {
            Text junk = _stream.readToDelimiterStringWithTimeout(">",
                    Stream::INCLUDE_DELIMITER, 5000); // throw away
        }
        incrementOffset(_stream.gcount());
    }

    template<typename PRIMITIVE> size_t serializeAny(const PRIMITIVE* object,
            const size_t count, const char* label)
    {
        size_t amount(count);

        if (get_direction() == OUT)
        {
            if (count > 1)
            {
                _stream << indent(_indent);
                _stream << "<primitive type=\"count\"" << "\" value=\""
                        << count << "\" />\n";
            }

            for (unsigned i(0); i < count; ++i)
            {
                _stream << indent(_indent);
                _stream << "<primitive type=\"" << ClassName(*object)
                        << "\" label=\"" << label << "\" value=\"" << object[i]
                        << "\" />\n";
            }

            incrementOffset(_stream.gcount());
        }
        else
        {
            Text xml_line = _stream.readToDelimiterStringWithTimeout(
                    ">", Stream::INCLUDE_DELIMITER, 5000);

            incrementOffset(xml_line.size());

            Text tag_type = ExtractXMLAttributeNameValue(xml_line,
                    "type");

            if (tag_type == "count")
            {
                unsigned amount = atoi(ExtractXMLAttributeNameValue(xml_line,
                        "value").c_str());

                for (unsigned i(0); i < amount; ++i)
                {
                    xml_line = _stream.readToDelimiterStringWithTimeout(">",
                            Stream::INCLUDE_DELIMITER, 5000);
                    incrementOffset(xml_line.size());
                    FromString(ExtractXMLAttributeNameValue(xml_line, "value"),
                            const_cast<PRIMITIVE&> (object[i]));
                }
            }
            else
            {
                FromString(ExtractXMLAttributeNameValue(xml_line, "value"),
                        const_cast<PRIMITIVE&> (object[0]));
            }
        }

        return amount;
    }

    size_t serializeAnyChar(const char* object, const size_t count,
            const char* label)
    {
        size_t amount(0);
        size_t number_of_elements(0);

        if (get_direction() == OUT)
        {
            _stream << indent(_indent);
            _stream << "<primitive type=\"string\" label=\"" << label << "\">\n";

            _stream << indent(++_indent) << "<![CDATA[";
	    number_of_elements = _stream.writeAll(count, object);
            amount += number_of_elements;
            _stream << "]]>\n";
            _stream << indent(--_indent) << "</primitive>\n";
            amount += _stream.gcount();
        }
        else
        {
            Text junk = _stream.readToDelimiterStringWithTimeout(">",
                    Stream::INCLUDE_DELIMITER, 5000);
            amount += junk.size(); // throw away opening tag

	    junk = _stream.readToDelimiterStringWithTimeout("<![CDATA[",
							    Stream::INCLUDE_DELIMITER, 5000);
	    amount += junk.size();

	    Text data = _stream.readToDelimiterStringWithTimeout("]]>",
								 Stream::NONE, 5000);
	    number_of_elements = data.size();
            amount += number_of_elements;

            ::strncpy(const_cast<char*> (object), data.c_str(), number_of_elements + 1);

            amount += _stream.readToDelimiterStringWithTimeout(">",
                    Stream::INCLUDE_DELIMITER, 5000).size(); // throw away nl after string

        }
        incrementOffset(amount);
        return number_of_elements;
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
