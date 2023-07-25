/*
Copyright 2009 by Walt Howard
$Id: Stream.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <MiniConfig.h>
#include <Exception.h>
#include <Misc.h>
#include <unistd.h>
#include <alloca.h>
#include <cstdarg>
#include <cstdio>
#include <ostream>
#include <vector>
#include <sstream>
#include <boost/shared_ptr.hpp>

/**
   Abstract base (which includes some actual functionality) for all Streamlike entities.
*/
class Stream
{
    struct Buffer
    {
        char* _buffer;
        size_t _buffer_size;
        char* _read_buffer; // char pointer to beginning of _buffer
        char* _end; // points to one past end of _buffer
        char* _read_point; // consumer of data will read from here.
        char* _insert_point; // new data arriving from stream will be put starting here.

        GETSET(size_t, _buffer_size);

	void Flush()
	{
	    _read_buffer = _buffer;
	    _read_point  = _buffer;
	    _insert_point = _buffer;
	    _buffer[0] = '\0';
	}

        Buffer(const size_t size) :
            _buffer(new char[2]), _buffer_size(2), _read_buffer(_buffer),
	    _read_point(_buffer), _insert_point(_buffer)
        {
            _buffer[0] = '\0';
            resize(size);
        }

        void resize(const size_t new_size)
        {
            char* new_buffer = new char[new_size + 1];
            ::memcpy(new_buffer, _buffer, std::min(_buffer_size, new_size));
            _read_buffer = new_buffer;
            _end = new_buffer + new_size;
            _read_point = new_buffer + (_read_point - _buffer);
            _insert_point = new_buffer + (_insert_point - _buffer);
            *_end = '\0';
            *_read_buffer = '\0';
            delete[] _buffer;
            _buffer = new_buffer;
            _buffer_size = new_size;
        }

        ~Buffer()
        {
            delete[] _buffer;
        }

    };

    MiniConfig _options;

    Text _resource;

    Text _option_string;

    boost::shared_ptr<Buffer> _buffer;

    size_t _gcount; // number of bytes written by the << operator since gcount() was last called.

    int _debug_file; // if not -1, copy writes out to this. Used only for debugging.

public:
    struct DelimiterList: public std::vector<const char*>
    {
	// Because of a limitation with va_arg ONLY primitive types can be used as ElementType in this function
	DelimiterList(unsigned count, const char* first_delimiter, ...) // Insert a list of ElementTypes into this container initially.
	{
	    va_list args;
	    va_start(args, first_delimiter);

	    push_back(first_delimiter);

	    while (count-- > 1)
	    {
		const char* temp = va_arg(args, const char*);
		push_back(temp);
	    }

	    va_end(args);
	}
    };

    size_t fillBuffer();

    enum OPTIONS
    {
        NONE, INCLUDE_DELIMITER
    };

    GETSET(MiniConfig, _options);
    GETSET(Text, _option_string);

    const Text& set_resource(const char* resource)
    {
	_resource = ((resource and *resource) ? resource : "resource not set");
	return _resource;
    }

    const Text& set_resource(const Text& resource)
    {
	return set_resource(resource.c_str());
    }

    GETTER(Text, _resource);

    void set_options(const char* options)
    {
	_options.loadFromNameValuePairs(options ? options : "");
    }

    // These must be overidden
    // read, write, open and close must all be NON-BLOCKING
    /**
       @brief - read data from a stream
       All subclasses who override this must maintain the following contract:
       @max_read Positive integer stating the maximum amount of data to read from the stream. read may return less than the max_read.
       @destination A char* where the read data will be stored.
       read returns the number of characters read, or -1 on error. set errno appropriately if your class does not automatically do it. set EAGAIN if
       the reason for the error is only that data is temporarily not available. return 0 to mean end of stream.
    */
    virtual size_t read(const size_t max_read, char* destination) = 0;

    /**
       @write - write data to a stream
       All clients who override this must maintain the following contract:
       @max_write   Positive integer stating the maximum amount of data to write to the stream. write may return less than the full amount in which case you must resend the remainder with another
       call to write.
       @source a char* points to the data that will be written
       write returns the number of characters written, or -1 on error. set errno appropriately if your class does not automatically do it. set EAGAIN if the reason for the error is only that
       data is temporarily not writeable
    */
    virtual size_t write(const size_t amount, const char* const source) = 0;

    // Return how many bytes have been read or written.
    virtual const unsigned long long& get_written() const = 0;

    virtual const unsigned long long& get_read() const = 0;

    /**
       Do not have to be implemented.
    */
    Stream(const char* resource, const char* options = "", const int size = 65534);

    virtual void open(const char* resource = NULL, const char* options = NULL);

    virtual Stream* accept();

    virtual void close();

    /**
       @breif  Check if eof has been reached (no more reading possible)
    */
    virtual bool eof();

    virtual void flush();

    virtual bool isWriteReady(const unsigned timeout_milliseconds = 0) = 0;

    virtual bool isReadReady(const unsigned timeout_milliseconds = 0) = 0;

    /**
	The following useful functions all use the virtual read/write and so work for all subclasses because the subclasses will use their versions of
	read and write.
    */
    virtual size_t writeAll(const size_t total_amount, const char* const original_source, int milliseconds_to_wait = 0);

    /** @brief   Performs a buffered, non-blocking read. Either reads ALL of the requested length, or none.
	@amount  How many bytes to read from the stream.
	@source  Where to put them.
	@return  lengthy of data if it was able to read the entire amount, 0 otherwise. No data is removed from the stream if 0 is read.
    */
    virtual size_t readAll(size_t amount, char* destination);

    /** @brief   Does a printf to the stream
	@format  A printf style format string.
	@...     A printf style list of arguments.
	@return  The number of bytes written. If a failure occurs, an exception is thrown. No -1 is ever returned.
	@throw   Exception
    */
    virtual size_t Printf(const char* format, ...)  __attribute__((format(printf, 2, 3)));

    virtual Text readString(const size_t max_length = 1024);

    virtual size_t writeString(const Text& write_me);

    /** @brief Reads stream up to and optionally including, any number of delimiter strings. Searches the input stream for all the specified
	delimiters and and the one that occurs earliest in the stream, is used.

	@delimiters std::vector of strings listing the delmiters we are interested in.
    */
    virtual Text readToDelimiterStrings(const Stream::DelimiterList& delimiters = DelimiterList(2, "\n", "\r"), const Stream::OPTIONS options = Stream::NONE);

    virtual Text readToDelimiterString(const char* delimiter = "\n", const Stream::OPTIONS options = Stream::NONE);

    const char* FirstDelimiter(const Stream::DelimiterList& delimiters);

    virtual size_t relay(const size_t atmost, Stream* output, const char* delimiter = "\n", const Stream::OPTIONS options = Stream::NONE);

    // Create a new, unopened version of me.
    virtual Stream* CopyNew() const = 0;

    /** brief  Return the number of characters written by the operator<<() since the last time this function was called.
     */
    virtual size_t gcount();

    virtual bool hasBuffered() const;

    virtual bool isFinite() const;

    virtual void resizeBuffer(const size_t newsize)
    {
	_buffer->resize(newsize);
    }

    virtual Text PeerAddress() const;

    virtual Text LocalAddress() const;

    virtual ~Stream();

    void increment_gcount(const size_t amount)
    {
        _gcount += amount;
    }
};

template<typename TYPE> Stream& operator<<(Stream& output, const TYPE& item)
{
    std::stringstream temp;
    temp << item;
    output.writeString(temp.str());
    output.increment_gcount(temp.str().size());
    return output;
}

template<typename STL_CONTAINER> std::vector<Stream*> HaveData(STL_CONTAINER& streams_to_check)
{
    std::vector<Stream*> readies; // list of streams which will be "read ready"

    for (typename STL_CONTAINER::const_iterator i(
	     streams_to_check.begin()); i != streams_to_check.end(); ++i)
    {
        if ((*i)->eof())
            continue;

        if ((*i)->hasBuffered()) // check to see if any buffered data has already been read.
            readies.push_back(*i); // we are read ready
        else
        {
            (*i)->fillBuffer(); // attempt to read some data (non-blocking).

            if ((*i)->hasBuffered()) // check to see if any buffered data has already been read.
                readies.push_back(*i);
        }
    }
    return readies;
}

template<template<typename> class STL_CONTAINER> std::vector<Stream*> HaveEof(STL_CONTAINER<Stream*>& streams_to_check)
{
    std::vector<Stream*> eofs; // list of streams which will be "eof" (end of file, no more reading possible)

    for (typename STL_CONTAINER<Stream>::const_iterator i(
	     streams_to_check.begin()); i != streams_to_check.end(); ++i)
    {
        if ((*i)->eof())
            eofs.push_back(*i);
    }

    return eofs;
}
