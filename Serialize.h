/*
   Copyright 2009 by Walt Howard
   $Id: Serialize.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <Exception.h>
#include <Channel.h>
#include <cstring>
#include <string>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <vector>
#include <unordered_map>
#include <Misc.h>
#include <errno.h>
#include <iostream>
#include <alloca.h>

/**
   Serialization Primitives

   There are a series of function named with a "Serialize" that help you perform writing and reading of your class to a different format than a C++
   structure. It's used where you want to transmit your class across a network connection, save it to disk for reconstitution etc.

   Because the "format" and media written to must be flexible, a Channel class is used. The Channel class does the work of writing C++ native types,
   which every class must at its most fundamental form, be composed of. You define the Channel class, or used one of the pre-defined ones. To do this
   you just subclass from another Channel class and rewrite the following virtual functions:

   virtual void SerializePrimitive(const unsigned char& primitive) = 0;
   virtual void SerializePrimitive(const signed char& primitive) = 0;
   virtual void SerializePrimitive(const unsigned short& primitive) = 0;
   virtual void SerializePrimitive(const signed short& primitive) = 0;
   virtual void SerializePrimitive(const unsigned int& primitive) = 0;
   virtual void SerializePrimitive(const signed int& primitive) = 0;
   virtual void SerializePrimitive(const unsigned long& primitive) = 0;
   virtual void SerializePrimitive(const signed long& primitive) = 0;
   virtual void SerializePrimitive(const unsigned long long& primitive) = 0;
   virtual void SerializePrimitive(const signed long long& primitive) = 0;
   virtual void SerializePrimitive(const float& primitive) = 0;
   virtual void SerializePrimitive(const double& primitive) = 0;
   virtual void SerializePrimitive(const bool& primitive) = 0;

   Every complex class must boil down eventually to a combo of those primitives and if the code knows how to serialize
   those, it can serialize whatever if composed of those (mostly).

   For example you may want to write to a network, to disk, to a memory location. You may want to write XML, or binary
   data, or string forms, or to a SQL database. Whatever format and media you want to use is handled by the Channel
   class.

   These templates help you serialize and deserialize your classes. At a minimum you need to define either a non-member
   function of this signature. The strange template <> syntax is necessary so that your function gets looked up as
   "specialization" of the more generic "template <typename CLASS> inline void Serialize(Channel&, const CLASS&, const
   char* label)"

   template <> inline void Serialize<YourClass>(Channel& channel, const YourClass& object)
   {
   if (channel.get_direction() == Channel::OUT)
   // Do whatever it takes to write the class out so that ....

   if (channel.get_direction() == Channel::IN)
   // Do whatever it takes to read in the class when it was written out ...
   }

   Or a member function of this signature:

   YourClass::Serialize(Channel& channel) const;

   class BongoBongo
   {
       int integer;
       std::string astring;
       std::list<Text> alistofstrings;
       void Serialize(Channel& channel) const
       { 
           Serialize(integer);  // Serialize Template figures out which function to use to serialize integers
           Serialize(astring);  // same for strings
           Serialize(alistofstrings);   // and stl objects
       }
   };

   That's it! That's all you need to do. Now your class can be streamed over any type of media, in any format.

   The "Channel" you use defines 1) The formats involved 2) the destination (network, file, memory etc) and 3) the
   direction. Are you serializing IN (setting the values of this object from a stream) or serializing OUT (delivering
   the object to someone else)

   If you don't have access to the source code of the class, you can define a non-member function to do the job:

   Example:

   class BongoBongo
   {
       int integer;
       Text astring;
       std::list<Text> alistofstrings;
   };

   void Serialize(const BongoBongo& bongo, Channel& channel) // <<== You need to define this
   {
       // Then each element of your class:
       Serialize(bongo.integer, channel);

       // Text is automatically supported though not strictly a primitive
       Serialize(bongo.astring, channel);

       // Even STL containers are supported
       Serialize(bongo.alistofstrings, channel);
   }

   BongoBongo myBongo;

   // This would save a bongo
   File output_file("FileWhereToSaveBongo.bin", "w");
   myBongo.serialize(output_file);

   // This would "reconstitute" that BongoBongo that was saved.
   File input_file("FileWhereToSaveBongo.bin", "r");
   myBongo.deserialize(input);

   Additional Factors:

   Your base class of course has to have it's own Serialize function.

   Serialization will apply itself recursively to any serializable data structure no matter how deep, as long
   as all the classes in the chain have the serialization members.

   The general idea here is that the Serialize template functions automatically instantiate to functions of the proper
   type to Serialize just about any type. If a class has a pointer member, the referent must also have a 
   Serialize function (or be a primitive). The Serialization library will also serialize it then. This is known
   as "template specialization" and it lets the simple "Serialize" template expand into other templates based
   upon the type involved. That makes it a lot easier to Serialize something since you don't need to be 
   concerned with its type. The Serilize<> template knows what to do with it automagically.

   Why is Serializing both in and out collapsed into the same function instead of having two functions like Serialize()
   and DeSerialize()?

   It is critical that data is Deserialized (read in) in the exact same order it was Serialized (written out). By using
   the same function for both reading and writing, there is no way the order can be inadvertantly swapped. Debugging a
   problem with non-human-readable binary data can be hell and everything should be done to prevent that situation
   happening.

   The Serialize functions have to do a few const_cast operations. This is because both serializing and deserializing
   are done in the same function. You want to be able to Serialize (Stream OUT) a const instance of a class so its
   Serialize function must be declared const. However, when used to Deserialize(Stream IN) data for form the instance
   of the class, it must be able to change the classes member variables.

   These functions use the BinaryIO functions at their lowest level so your class will be serialized as binary data, not
   very human readable.
*/

/** @brief  Throws exception giving a lot of information related to serializing.
 *   @param  file       The  File object being streamed to/from.
 *   @param  direction  Direction of information flow, IN = deserializing, OUT = serializing.
 *   @param  message    The error message composed by the programmer
 */
void ThrowSerializationException(const Channel& channel, const Text& message, int errnum = Exception::ERRNO_IGNORE);

/**
   Used at the beginning of serialization to either write out, or read in a name and version for the class being serialized. You don't have to use the
   actual name of the class as "name" but make sure whoever is deserializing knows what to deserialize when it sees the name. If
   channel.get_direction() is in, class_naame and current_version will be modified to contain the name and version of the serialized data about to be
   read in. This also returns the version number as that is useful in certain applications.
*/
float SerializeClassNameAndVersion(Channel& channel, Text& class_name, float& current_version);

/**
 *  @brief   The "SerializePointer" function serializes a pointer without attempting to dereference the
 *           referent. Sometimes you only want to store the pointer.
 */
template <typename CLASS> void SerializePointer(Channel& channel, CLASS*& pointer, const char* label);

/**
 *  @brief PointerOrReference - Homogenizes pointers and references.
 *         From dictionary.com: 3.to make uniform or similar, as in composition or function.
 * 	  Constructs with either a pointer or reference but acts like a reference to "Class"
 *  @note  C++ template functions are unable to broadly specialize between "any class" and
 *         "that class's pointer". Template classes can. This class helps the general Serialize
 *         function work for both pointers to objects and references to objects.
 */
template<typename CLASS> class PointerOrReference
{
    CLASS& _item_ref;

public:
    explicit PointerOrReference(CLASS& item) :
        _item_ref(item)
    {
    }

    void serialize(Channel& channel, const char* label)
    {
        Text classname = ClassName(_item_ref);

        if (not channel.get_open())
	    ThrowSerializationException(channel, "You need to call the open() member of your Channel function.");

        // we need this here to mark this stack frame as the beginning of the serialization. serialization is recursive so when we exit this function,
        // it means we serialized out all the members. We serialized everything asked for mark the Channel as not open. This enforces a
        // synchoronization so we don't end up reading an channel except at the beginning of an object packaged in the channel.
        channel.set_open(channel.get_open() + 1);

        /** execute any "start of class" close the channel defines */
        channel.startOfClass(ClassName(_item_ref).c_str(), label);

        /** execute the actual class defined serialization code */
        _item_ref.serialize(channel, label); //.serialize(channel, label);

        /** execute any "start of class" close the channel defines */
        channel.endOfClass(ClassName(_item_ref).c_str(), label);

        // After the entire first object is streamed out and all the recursion is unwound we end up back here in the original call frame which started this all off. At this point
        // we terminate the channel and mark it as closed so it cannot be reused by accident. This forces the caller to call open() again (which should tell him the name of the next class which
        // is in the stream so he can deserialize properly)
	channel.set_open(channel.get_open() - 1);
	if (channel.get_open() == 0)
	{
            channel.set_open(0);
            channel.close();
        }
    }
};

/**
 *  @brief  Serializing a pointer is very different from serializing an actual object. This class
 *          is a specialization of the default PointerOrReference. This will construct if a pointer data
 *          type is attempting serialization.
 *  @note   This is somewhat complicated because the pointer in the deserialize call must sometimes be modified
 *          in the caller's space. That is why we use a reference to a pointer. It also understands if pointers point
 *          to the same object and only stores or restores one copy of the common object.
 */
template<typename CLASS> class PointerOrReference<CLASS*>
{
    typedef CLASS* ClassPtr;
    ClassPtr& _item;

public:
    PointerOrReference(ClassPtr& item) :
        _item(item)
    {
    }

    /**
     *  Pointers never have their own serializers. They must rely on the serialization of their referent
     *  class. This will recurse back through the "Serialize" functions now looking for Class&  instead of
     *  Class* . We save the pointer value also so we can save the fact that we had a NULL pointer, and we
     *  can also deserialize shared pointers to point to the same object if their original pointer values
     *  were the same.
     */
    void serialize(Channel& channel, const char* label);
    /**
       Definition for this function is way at the bottom of this file.
    */
};

/** @brief  Use this when serializing a base class. This function helps avoid a bit of mess when seralizing base
 *           classes, Example:
 *           Serialization::SerializeBase<YourBaseClassType>(file, direction,* this);
 *           Of course, the base class itself must also have a Serialize(Channel&, BaseClass&) function.
 *   @param  file       The  File object being streamed to/from.
 *   @param  direction  Specifies whether this is a serialization or deserialization.
 *   @param  item       The item being Serialized.
 */
template<typename CLASS> inline void SerializeBase(Channel& channel, CLASS& item, const char* label = NULL)
{
    Serialize(channel, item, label);
}

/**
 *  @brief   THIS IS THE CORE WORKHORSE FUNCTION OF THIS ENTIRE CODEBASE
 *           The general "Serialize" function which gets called if your class is not one of the built in primitives or
 *           STL containers.
 *  @param   channel                The Channel object used to determine format and medium for serialization.
 *  @param   pointer_or_reference   The item being serialized.
 */
template<typename CLASS> inline void Serialize(Channel& channel, CLASS& item, const char* label = NULL)
{
    if (channel.get_direction() > Channel::OUT or channel.get_direction() < Channel::IN)
        ThrowSerializationException(channel, "Invalid direction (not OUT nor IN)");

    /** C++ templates FUNCTIONS cannot differentiate between pointers to a class, and instances of a class, but template CLASSES can. This utility
	class PointerOrReference does this, automatically detecting a pointer being serialized, and serializing out its referent
	PointerOrReference<CLASS> item(pointer_or_reference);
    */
    PointerOrReference<CLASS> pointer_or_reference(item);

    pointer_or_reference.serialize(channel, label ? label : ClassName(item).c_str());
}

/**
   @brief  These are "specialized" templates that are instantiated instead of the general "void Serialize(File&,
   CLASS&, Direction) when the item being Serialized is one of the basic c++ types.
   You can add any popular types here such as Text. These do not call StartOfClass() or EndOfClass().
*/
template<> inline void Serialize<bool> (Channel& channel, bool& item, const char* label)
{
    channel.serializeBool(&item, 1, label);
}

template<> inline void Serialize<double> (Channel& channel, double& item, const char* label)
{
    channel.serializeDouble(&item, 1, label);
}

template<> inline void Serialize<float> (Channel& channel, float& item, const char* label)
{
    channel.serializeFloat(&item, 1, label);
}

template<> inline void Serialize<char> (Channel& channel, char& item, const char* label)
{
    channel.serializeChar(&item, 1, label);
}

template<> inline void Serialize<unsigned char> (Channel& channel, unsigned char& item, const char* label)
{
    channel.serializeUnsignedChar(&item, 1, label);
}

template<> inline void Serialize<short> (Channel& channel, short& item, const char* label)
{
    channel.serializeShort(&item, 1, label);
}

template<> inline void Serialize<unsigned short> (Channel& channel, unsigned short& item, const char* label)
{
    channel.serializeUnsignedShort(&item, 1, label);
}

template<> inline void Serialize<int> (Channel& channel, int& item, const char* label)
{
    channel.serializeInt(&item, 1, label);
}

template<> inline void Serialize<unsigned int> (Channel& channel, unsigned int& item, const char* label)
{
    channel.serializeUnsignedInt(&item, 1, label);
}

template<> inline void Serialize<long> (Channel& channel, long& item, const char* label)
{
    channel.serializeLong(&item, 1, label);
}

template<> inline void Serialize<unsigned long> (Channel& channel, unsigned long& item, const char* label)
{
    channel.serializeUnsignedLong(&item, 1, label);
}

template<> inline void Serialize<long long> (Channel& channel, long long& item, const char* label)
{
    channel.serializeLongLong(&item, 1, label);
}

template<> inline void Serialize<unsigned long long> (Channel& channel, unsigned long long& item, const char* label)
{
    channel.serializeUnsignedLongLong(&item, 1, label);
}

// If the item is const, unconst it.
template<typename CLASS> inline void Serialize(Channel& channel, const CLASS& item, const char* label)
{
    Serialize(channel, const_cast<CLASS&>(item), label);
}

template<typename ELEMENT> inline size_t SerializeArray(Channel& channel, ELEMENT* array, const size_t& number_of, const char* label)
{
    throw Exception(LOCATION, StringPrintf(0, "This cannot be expanded. You need to write your own specialization class for: \"%s\". "
					   "Beware of serializing const*", ClassName(array).c_str()));
}

template<> inline size_t SerializeArray<char> (Channel& channel, char* array, const size_t& number_of, const char* label)
{
    return channel.serializeChar(array, number_of, label);
}

/** Serialize an array of chars */
template<> inline size_t SerializeArray<unsigned char> (Channel& channel, unsigned char* array, const size_t& number_of, const char* label)
{
    return channel.serializeUnsignedChar(array, number_of, label);
}

/**
   @note   It helps greatly to write serialization template functions for some standard types that are used everywhere,
   specifically, Text, std::pair and all the STL containers.
*/
template<> inline void Serialize<std::string> (Channel& channel, std::string& item, const char* label)
{
    static const size_t buffersize(1000000); // I know, I know, this is huge but it's on the stack so only temporary
    char* buffer = static_cast<char*> (alloca(buffersize));

    if (channel.get_direction() == Channel::OUT)
    {
        if (item.size() > buffersize)
            ThrowSerializationException(channel, StringPrintf(0, "Cannot stream Text larger than: %zd", buffersize),
					Exception::ERRNO_IGNORE);
        SerializeArray(channel, const_cast<char*>(item.c_str()), item.size(), label);
    }
    else
    {
        size_t length = SerializeArray(channel, buffer, buffersize, label);
        const_cast<std::string&> (item) = std::string(buffer, length);
    }
}

template<> inline void Serialize<Text> (Channel& channel, Text& item, const char* label)
{
    static const size_t buffersize(1000000); // I know, I know, this is huge but it's on the stack so only temporary
    char* buffer = static_cast<char*> (alloca(buffersize));

    if (channel.get_direction() == Channel::OUT)
    {
        if (item.size() > buffersize)
            ThrowSerializationException(channel, StringPrintf(0, "Cannot stream Text larger than: %zd", buffersize),
					Exception::ERRNO_IGNORE);
        SerializeArray(channel, const_cast<char*>(item.c_str()), item.size(), label);
    }
    else
    {
        size_t length = SerializeArray(channel, buffer, buffersize, label);
        const_cast<Text&> (item) = Text(buffer, length);
    }
}

/** Enums need special handling since templates can't detect "enum" as a type.*/
template<typename EnumType> inline void SerializeEnum(Channel& channel, EnumType& item, const char* label)
{
    // Conversion FROM enum TO int is ok
    int temp(item);

    // writing and reading the int value is easy, the enum automagically converts to an it
    channel.serializeInt(&temp, 1, label);

    // Reading is more difficult int -> enum is not an automatic conversion and needs to be forced.
    // Conversion FROM int TO enum needs to be forced with a static_cast
    if (channel.get_direction() == Channel::IN)
    {
        EnumType enum_value = static_cast<EnumType> (temp);
        const_cast<EnumType&> (item) = enum_value;
    }
}

template<typename FIRST, typename SECOND> inline void Serialize(Channel& channel, std::pair<FIRST, SECOND>& item, const char* label)
{
    channel.startOfClass(ClassName(item).c_str(), label);
    FIRST& first = const_cast<FIRST&>(item.first);
    Serialize(channel, first, "first");
    Serialize(channel, item.second, "second");
    channel.endOfClass(ClassName(item).c_str(), label);
}

template<typename StlContainer> void SerializeContainer(Channel& channel, StlContainer& cont, const char* label)
{
    channel.startOfClass(ClassName(cont), label);
    StlContainer& container = const_cast<StlContainer&> (cont);
    uint64_t element_count;
    if (channel.get_direction() == Channel::IN)
    {
        Serialize(channel, element_count, "count");
        container.clear(); // Clean out any pre-existing values
        for (uint64_t i = 0; i < element_count; ++i)
        {
            // Redundant? No. This assignment is necessary to make sure objects
            // especially pointers are set to default state. Simple types are not initialized
            // to default by simply declaring them. They must be assigned like int i = int();
            typename StlContainer::value_type item = typename StlContainer::value_type();
            Serialize(channel, item, "member");
            container.insert(container.end(), item);
        }
    }
    else
    {
        element_count = container.size();
        Serialize(channel, element_count, "count");
        for (typename StlContainer::iterator item(container.begin()); item
		 != container.end(); ++item)
            Serialize(channel, *item, "member");
    }
    channel.endOfClass(ClassName(cont).c_str(), label);
}

template<typename Element> inline void Serialize(Channel& channel, std::vector<Element>& container, const char* label)
{
    SerializeContainer(channel, container, label);
}

template<typename Key, typename Value> inline void Serialize(Channel& channel, std::map<Key, Value>& container, const char* label)
{
    SerializeContainer(channel, container, label);
}

template<typename Key, typename Value> inline void Serialize(Channel& channel, std::unordered_map<Key, Value>& container, const char* label)
{
    SerializeContainer(channel, container, label);
}

template<typename Key, typename Value> inline void Serialize(Channel& channel, std::multimap<Key, Value>& container, const char* label)
{
    SerializeContainer(channel, container, label);
}

template<typename Element> inline void Serialize(Channel& channel, std::set<Element>& container, const char* label)
{
    SerializeContainer(channel, container, label);
}

template<typename Element> inline void Serialize(Channel& channel, std::multiset<Element>& container, const char* label)
{
    SerializeContainer(channel, container, label);
}

template<typename Element> inline void Serialize(Channel& channel, std::list<Element>& container, const char* label)
{
    SerializeContainer(channel, container, label);
}

template<typename Element> inline void Serialize(Channel& channel, std::deque<Element>& container, const char* label)
{
    SerializeContainer(channel, container, label);
}

template <typename CLASS> void SerializePointer(Channel& channel, CLASS*& pointer, const char* label)
{
/**
 *  Because there are differences in pointer size between 32 and 64 bit platforms, we have to perform
 *  some translation here. We cannot serialized directly into pointer because our
 *  SerializedPointerFormat is always 64 bits long which would overflow a 32 bit pointer value if
 *  assigned directly.
 */
    Channel::SerializedPointerFormat temp;

    if (channel.get_direction() == Channel::OUT)
	temp = reinterpret_cast<Channel::SerializedPointerFormat> (pointer);

    ::Serialize(channel, temp, label);

    if (channel.get_direction() == Channel::IN)
    {
	pointer = reinterpret_cast<CLASS*> (temp);
    }
}

/**
 *  Pointers never have their own serializers. They must rely on the serialization of their referent
 *  class. This will recurse back through the "Serialize" functions now looking for Class&  instead of
 *  Class* . We save the pointer value also so we can save the fact that we had a NULL pointer, and we
 *  can also deserialize shared pointers to point to the same object if their original pointer values
 *  were the same.
 */
template<typename CLASS> void PointerOrReference<CLASS*>::serialize(Channel& channel, const char* label)
{
    if (channel.get_direction() == Channel::OUT)
    {
        // Serialize the actual pointer. It will be saved as a uint_64 handle. Though not really useful as a memory location, it will serve to identify cases
        // where pointers point to the same object. The referent of the pointer will also be saved.
        SerializePointer(channel, _item, label);

        // No need to save referents of NULL pointers
        if (_item == NULL)
            return;

        // See if we have already saved this pointer
        if (channel.get_pointer_map().find(
                reinterpret_cast<Channel::SerializedPointerFormat> (_item))
	    != channel.get_pointer_map().end())
            return;

        // Dereference the referent and serialize it in its entirety
        ::Serialize(channel, *_item, label);

        // store this pointer in the pointer_map so we can identify if another pointer to the same object is attempting serialization.
        channel.get_pointer_map()[reinterpret_cast<Channel::SerializedPointerFormat> (_item)] = _item;
    }
    else
    {
        /**
         *  Warning: There is a prerequisite here. The pointer being reconstituted must be either have been NULL, or pointer to a valid instance of
         *  CLASS when it was serialized out. It must not be an uninitialized pointer. If you get a crash here it is probably because you are
         *  trying to deserialize into an uninitialized pointer. Make sure when you streamed OUT (serialized) this thing, it wasn't serialized with
         *  an uninitialized pointer.
         */

        // First read in the pointer that was saved.
        void* restored_pointer = 0;
        SerializePointer(channel, restored_pointer, label);

        // Source and target both NULL? Nothing to do.
        if (restored_pointer == NULL and _item == NULL)
            return;

        /**
         *   Now, determine what to read in next.
         */
        ClassPtr& target_item = _item;

        /**
         *  Is the source NULL, but the target is not? Delete the target and set
         *  its pointer to NULL. It means the saved object had this pointer set to NULL
         *  when it was serialized and we want to restore that saved state.
         */
        if (restored_pointer == NULL)
        {
            if (target_item != NULL)
                ThrowSerializationException(channel,
					    StringPrintf(0, "Your class must manage setting this pointer (%s) member (of class %s) to NULL before serializing in: ",
							 label, ClassName(_item).c_str()));
            target_item = NULL;
            return;
        }

        /** Has this pointer referent already been restored?*/
        typename Channel::PointerToPointer::const_iterator ptr_iter =  channel.get_pointer_map().find(reinterpret_cast<Channel::SerializedPointerFormat> (restored_pointer));

        // If we've already restored this object
        if (ptr_iter != channel.get_pointer_map().end())
        {
            target_item = reinterpret_cast<ClassPtr> (ptr_iter->second);
            return;
        }

        /**
         *  If the target pointer is NULL but we need to deserialize an actual object,
         *  construct a new default object up in there into which we will deserialize. We
         *  don't know how to deserialize anything. We must construct something and then
         *  have that something deserialize itself.
         */
        if (target_item == NULL)
            target_item = new CLASS();

        ::Serialize(channel, *target_item, label);

        // We have now reconstituted this, add it to the pointer map
        channel.get_pointer_map()[reinterpret_cast<Channel::SerializedPointerFormat> (restored_pointer)] = target_item;
    }
}

template<typename OBJECT> void SendObject(const Channel& channel, OBJECT& object)
{
    // These const casts are ugly but the only way to work around the C++ "bug" of flagging non-const temporary as an error
    const_cast<Channel&> (channel).open(ClassName(object).c_str());
    SERIALIZE(const_cast<Channel&>(channel), object);
}

template<typename OBJECT> OBJECT& RecvObject(const Channel& channel, OBJECT& object)
{
    // These const casts are ugly but the only way to work around the C++ "bug" of flagging non-const temporary as an error
    SERIALIZE(const_cast<Channel&>(channel), object);
    return const_cast<OBJECT&> (object);
}
