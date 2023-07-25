/*
   Copyright 2009 by Walt Howard
   $Id: Serialize.cc 2428 2012-08-14 15:33:13Z whoward $
*/


#include <Serialize.h>
#include <Text.h>
#include <cstdlib>

/**
 *  @brief   The "SerializePointer" function serializes a pointer without attempting to dereference the
 *           referent. Sometimes you only want to store the pointer.
 */
/**
 *  Because there are differences in pointer size between 32 and 64 bit platforms, we have to perform
 *  some translation here. We cannot serialized directly into pointer because our
 *  SerializedPointerFormat is always 64 bits long which would overflow a 32 bit pointer value if
 *  assigned directly.
 */
/*
void SerializePointer(Channel& channel, void*& pointer, const char* label)
{
    Channel::SerializedPointerFormat temp;

    if (channel.get_direction() == Channel::OUT)
        temp = reinterpret_cast<Channel::SerializedPointerFormat> (pointer);

    Serialize(channel, temp, label);

    if (channel.get_direction() == Channel::IN)
    {
        pointer = reinterpret_cast<void*> (temp);
    }
}
*/

float SerializeClassNameAndVersion(Channel& channel, Text& class_name, float& current_version)
{
    if (channel.get_direction() == Channel::OUT)
    {
        Text class_name_and_version(StringPrintf(0, "%s-%f", class_name.c_str(), current_version));
        Serialize(channel, class_name_and_version, "class-version");
        return current_version;
    }

    //
    // If we make it here, direction == Channel::IN
    //

    Text class_name_and_version;
    Serialize(channel, class_name_and_version, "class-version");

    if (const char* version = strchr(class_name_and_version.c_str(), '-'))
    {
        Text classname(class_name_and_version.c_str(), version
		       - class_name_and_version.c_str() - 1);
        current_version = atof(version + 1);
        return current_version;
    }

    ThrowSerializationException(channel, StringPrintf(0, "Missing version number in Serialized class: %s", class_name_and_version.c_str()));

    return 0; // Only here to keep compiler happy.
}

Text SerializeClassName(Channel& channel, const char* class_name = NULL)
{
    if (channel.get_direction() == Channel::OUT)
    {
        int length(strlen(class_name));
        channel.serializeChar(const_cast<char*>(class_name), length, "class_name");
        return "";
    }
    else
    {
        char buffer[128] = "";
        int length = sizeof(buffer) - 1;
        channel.serializeChar(buffer, length, "class_name");
        return buffer;
    }
}

/** @brief  Throws exception giving a lot of information related to serializing.
 *   @param  file       The libiViaCore File object being streamed to/from.
 *   @param  direction  Direction of information flow, IN = deserializing, OUT = serializing.
 *   @param  message    The error message composed by the programmer
 */
void ThrowSerializationException(const Channel& channel, const Text& message, int errnum)
{
    Text msg = StringPrintf(0, "%s (while %s at offset %ld)", message.c_str(), Channel::IN ? "serializing in" : "serializing out", channel.get_offset());
    throw Exception(LOCATION, msg.c_str(), errnum);
}
