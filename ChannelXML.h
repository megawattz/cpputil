/*
Copyright 2009 by Walt Howard
$Id: ChannelXML.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

/** @brief Write/Read classes as XML text
 */

#include <Channel.h>
#include <Stream.h>

class ChannelXML: public Channel
{
    Stream& _stream;
    unsigned _indent;

    Text indent(const int amount)
    {
        return Text("                                                                                        ", amount * 4);
    }

public:
    ChannelXML(const Channel::DIRECTION& direction, Stream& stream) :
	Channel(direction), _stream(stream), _indent(0)
    {
    }

    virtual Text open(const char* package_name = NULL)
    {
        if (get_direction() == Channel::OUT)
        {
            if (not package_name)
                throw Exception(LOCATION, "You must provide a package name when serializing OUT");
            set_open(1);
            _stream << "<package type=\"" << package_name << "\" />\n";
            return package_name;
        }
        else
        {
            set_open(1);
            Text package_line(_stream.readToDelimiterString("\n", Stream::INCLUDE_DELIMITER));
            return ExtractXMLAttributeNameValue(package_line, "type");
        }
    }

    Text ExtractXMLAttributeNameValue(const Text& xml, const char* tag)
    {
        const char* tagstart = strstr(xml.c_str(), tag);

        if (not tagstart)
            return "";

        char value[128] = "";
        // read and throw away everything that is not an equal sign. Then throw away anything that IS an equal sign, whitespace or a quote. Then read the value which is everything up to the next quote.
        int rval = sscanf(tagstart, "%*[^=]%*[= \t\"]%127[^\"]", value);
        if (rval != 1)
            throw Exception(LOCATION, "Unable to get value of tag");

        return value;
    }

    virtual void startOfClass(const char* classname, const char* label)
    {
        if (get_direction() == Channel::OUT)
        {
            _stream << indent(_indent++);
            _stream << "<class type=\"" << classname << "\" label=\"" << label
                    << "\">\n";
        }
        else
        {
            Text junk = _stream.readToDelimiterString(">", Stream::INCLUDE_DELIMITER); // throw away
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
            Text junk = _stream.readToDelimiterString(">", Stream::INCLUDE_DELIMITER); // throw away
        }
        incrementOffset(_stream.gcount());
    }

    template<typename PRIMITIVE> size_t serializeAny(PRIMITIVE* object, const size_t count, const char* label)
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
            Text xml_line = _stream.readToDelimiterString(">", Stream::INCLUDE_DELIMITER);

            incrementOffset(xml_line.size());

            Text tag_type = ExtractXMLAttributeNameValue(xml_line, "type");

            if (tag_type == "count")
            {
                unsigned amount = atoi(ExtractXMLAttributeNameValue(xml_line, "value").c_str());

                for (unsigned i(0); i < amount; ++i)
                {
                    xml_line = _stream.readToDelimiterString(">", Stream::INCLUDE_DELIMITER);
                    incrementOffset(xml_line.size());
                    FromString(ExtractXMLAttributeNameValue(xml_line, "value"), const_cast<PRIMITIVE&> (object[i]));
                }
            }
            else
            {
                FromString(ExtractXMLAttributeNameValue(xml_line, "value"), const_cast<PRIMITIVE&> (object[0]));
            }
        }

        return amount;
    }

    size_t serializeAnyChar(char* object, const size_t count, const char* label)
    {
        size_t amount(0);
        size_t number_of_elements(0);

        if (get_direction() == OUT)
        {
            _stream << indent(_indent);
            _stream << "<primitive type=\"" << ClassName(*object)
                    << "\" label=\"" << label << "\" count=\"" << count
                    << "\">\n";
            _stream << indent(++_indent) << "<![CDATA[";
	    number_of_elements = _stream.writeAll(count, object);
	    amount += number_of_elements;
            _stream << "]]>\n";
            _stream << indent(--_indent) << "</primitive>\n";
            amount += _stream.gcount();
            number_of_elements = count;
        }
        else
        {
            Text junk = _stream.readToDelimiterString(">", Stream::INCLUDE_DELIMITER);
            unsigned count = atoi(ExtractXMLAttributeNameValue(junk, "count").c_str());
            amount += junk.size(); // throw away opening tag
            Text data;
            if (count > 0) // readToDelimiter will think empty string is "no data" so we can't call it if count is 0.
            {
                junk = _stream.readToDelimiterString("<![CDATA[", Stream::INCLUDE_DELIMITER);
                amount += junk.size();
                data = _stream.readToDelimiterString("]]>", Stream::NONE);
            }
            else
            {
                //just throw away line and leave data as empty string.
                junk = _stream.readToDelimiterString("]]>", Stream::INCLUDE_DELIMITER);
            }
            number_of_elements = data.size();
            amount += number_of_elements;
            ::strncpy(object, data.c_str(), data.size() + 1);
            amount += _stream.readToDelimiterString(">", Stream::INCLUDE_DELIMITER).size(); // throw away nl after string
        }
        incrementOffset(amount);
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
