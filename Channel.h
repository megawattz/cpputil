/*
Copyright 2009 by Walt Howard
$Id: Channel.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <map>
#include <stdint.h>
#include <Misc.h>
#include <Exception.h>

/**
 A channel class defines where the serialization goes, where it comes from and the storage format. You can write your
 own, though the provided one "FileChannel" can be used for sort of I/O that uses a file descriptor. All it has to do is
 read and write the C++ primitive types, int, unsigned int, etc.  And raw arrays of such (to support char* style data)
 */

int ItemMetaType(const char* class_name);

class Channel
{
public:
    enum DIRECTION
    {
        IN = 1, OUT = 2
    };

    /*
      Though you can serialize by decomposing everything down to primitives recursively, sometimes that isn't too readable. Some types are better
      representing by shortcutting and displaying them a bit more high level like a string as 
      READABLE: "this is a string" 
      instead of as an array which is how a string is stored internally 
      UNREADABLE ['t','h','i','s',' ','i','s',' ','a',' ','s','t','r','i','n','g']. 
      
      The Function MetaType will return what overall type the requested type is. Then you can write you Channel to serialize them with an 
      understanding of the type. This list can be added to as more interesting types of classes get serialized. For example, a map is 
      probably better displayed as

      items:
      {
      0: "In the councils of government",
      1: "we must guard",
      2: "against the acqusition"
      }

      Instead of

    items:
    [
        count: 10,
        member:
        {
            first: 0,
            second:
            {
                second:
                [
                    count: 14,
                    member: "In",
                    member: "the",
                    member: "councils",
                    member: "of",
                    member: "government,",

		    etc.....
    */

    enum META_TYPE
    {
	UNINTERESTING, // No special meta type assigned.
	PRIMITIVE,     // native language types
	ASSOCIATIVE_UNIQUE,   // an associative container, key value pairs with unique keys
	ASSOCIATIVE_MULTI, // associated container with keys that can be duplicates
	SEQUENTIAL,    // Single element containers (the single elements can be complex of course)
	TRANSPARENT,   // Helper classes that don't really need to be explicitly serialize like Text (which is a subclass of std::string without additional members)
	TREE,          // A tree structure
	PAIR          // Items like the STL pair.
    };

    META_TYPE MetaType(const char* class_name);

private:
    /**
     *  This following section is used to keep track of pointer values while serializing and deserializing.  The idea
     *  here is, when serializing pointers to objects, the actual pointer values become meaningless because when the
     *  object is re-constituted on the receiver end, the memory layout is different. However, that is not a problem as
     *  long as and entire structure of data with interlinked pointers is serialized as ONE object. This library will
     *  figure out all the pointer/reference relationships. When we serialize out a pointer we do keep track
     *  of its value. The absolute value of the pointer is irrelevant, but if another pointer has the same value, we
     *  know that when we deserialize things, we can set pointers that were identical originally, back to pointing at
     *  the identical address. For example, reference counted pointers need this capability if they are to be restored
     *  in the same state.
     */

    /**
     *  This map keeps track of the pointers which have been serialized. If the same pointer value is serialized more than once, it means that pointer
     *  was pointing to the same object as a previous pointer. We need to keep track of this so when we deserialize we can restore the fact that these
     *  pointers were pointing to the same object.
     */

public:
    typedef uint64_t SerializedPointerFormat; /** we will save pointers as uint64_t top make sure we accomodate even 64 bit architectures*/
    typedef std::map<SerializedPointerFormat, void*> PointerToPointer;

private:
    PointerToPointer _pointer_map;
    DIRECTION _direction;
    uint64_t _offset; // For debugging - how far into the stream an error occurred
    int _open;

protected:
    uint64_t incrementOffset(uint64_t amount)
    {
        _offset += amount;
        return _offset;
    }

public:
    Channel::PointerToPointer& get_pointer_map()
    {
        return _pointer_map;
    }

    GETSET(DIRECTION, _direction);
    GETSET(uint64_t, _offset);

    const int& get_open() const
    {
        return _open;
    }

    const int& set_open(const int& new_value)
    {
        if (_open == 1 and new_value == 1)
            throw(Exception(LOCATION, "Channel already opened."));

        if (_open == 0 and new_value == 0)
            throw(Exception(LOCATION, "Channel already closed."));

        _open = new_value;

        return _open;
    }

    Channel(const Channel::DIRECTION& direction);

    /**
     Any channel class must only know how to stream in and out the C++ primitive types Your particular Channel class will have to override these
     functions: NOTE: when _direction is IN it means the parameters are serialized in, written to. You must cast away const in your functions when
     _direction is IN. This seems unsafe but without doing that another can of worms is opened, mainly Serialization gets more complicated. Then
     label parameter is optionally used if you want to label the data written, for example if you are streaming out XML. All code that serializes
     should tag the data with a label because you don't know that later on a particular Channel class won't want to use that data when it
     reads/writes. You need to label the data always, because you don't know what "Channel" class someone else will apply that WILL want the label
     data.
     */

    virtual size_t serializeChar(char*, const size_t& count, const char* label) = 0;
    virtual size_t serializeUnsignedChar(unsigned char*, const size_t& count, const char* label) = 0;

    virtual size_t serializeShort(short int*, const size_t& count, const char* label) = 0;
    virtual size_t serializeUnsignedShort(unsigned short int*, const size_t& count, const char* label) = 0;

    virtual size_t serializeInt(int*, const size_t& count, const char* label) = 0;
    virtual size_t serializeUnsignedInt(unsigned int*, const size_t& count, const char* label) = 0;

    virtual size_t serializeLong(long*, const size_t& count, const char* label) = 0;
    virtual size_t serializeUnsignedLong(unsigned long*, const size_t& count, const char* label) = 0;

    virtual size_t serializeLongLong(long long*, const size_t& count, const char* label) = 0;
    virtual size_t serializeUnsignedLongLong(unsigned long long*, const size_t& count, const char* label) = 0;

    virtual size_t serializeFloat(float*, const size_t& count, const char* label) = 0;
    virtual size_t serializeDouble(double*, const size_t& count, const char* label) = 0;

    virtual size_t serializeBool(bool*, const size_t& count, const char* label) = 0;

    /**
     Whenever a non-trivial class is serialized, these functions are called before and after serializing it. You do not have to use them but if you
     are doing something like serializing XML, you'll have to have your Channel class "open a tag" before serializing the object and "close the tag"
     when finished, perhaps even indenting and outdenting to make the serialized data more readable. See the Channel.XML.h file for explanation
     */
    virtual void startOfClass(const char* classname, const char* label); // called before streaming any non-simple class. Useful for inserting class wrapper data like XML opening tags, etc.

    virtual void endOfClass(const char* classname, const char* label); // called just after streaming any non-simple class. These are not always needed.

    /**
     Before serializing, you must call "open". If you are writing a channel, you must write into the beginnig of the channel with the name of the
     class you are serializing. When you are serializing in, pass NULL and open will return the class you must deserialize. Your overloaded open and
     close functions must emulate this behavior. Deserialization must be able to know WHAT class was serialized in this channel.

     When deserializing something, the Serialize function will reconstitute the full class that was originally serialized so you don't have to label
     each member of the class. It's only when the objects coming at you are unknown ahead of time that you have to.

     So your open() function has to write the class name when _direction is out, and read the class name when _direction is in. The special function
     SerializeClassName() will assist you in this. These defaults provided in the base class should work for Channel that are simple raw binary
     streams of data.

     IMPORTANT: open() and close() apply to the logical aspect of an Channel NOT the physical aspect. You can open() and close() a Channel multiple
     times for example while serializing to the same file, WITHOUT OPENING AND CLOSING THE FILE. The open() and close() serve to demark the objects
     that are in the stream. They don't open and close the physical stream.
     */
    virtual Text open(const char* package_name = NULL) = 0;

    virtual void close(const char* = NULL)
    {
    }
    ;

    Text nextPackage(const char* package_name = NULL)
    {
        if (_direction == Channel::IN)
            return open();
        else
            return open(package_name);
    }

    virtual ~Channel();
};

/**
 Convenience Macro which will expand out to serializing the variable AND passes the NAME of the variable as a string
 */
#define SERIALIZE(CHANNEL, VARIABLE)  ::Serialize(CHANNEL, VARIABLE, #VARIABLE)
