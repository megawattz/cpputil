/*
Copyright 2009 by Walt Howard
$Id: FileDescriptorStream.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <Stream.h>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <map>
#include <fcntl.h>
#include <Misc.h>
#include <MiniConfig.h>

std::vector<int> AreDescriptorsReadReady(const unsigned timeout_milliseconds, const std::vector<int>& fds_to_check);

std::vector<int> AreDescriptorsWriteReady(const unsigned timeout_milliseconds, const std::vector<int>& fds_to_check);

template<typename STREAM> std::vector<STREAM*> AreReadReady(const unsigned timeout_milliseconds, std::vector<STREAM*>& streams_to_check)
{
    std::vector<int> check_these; // We will need to get the actual descriptors of all the Streams into a vector.

    std::map<int, STREAM*> reverse_map; // used later to quickly build a vector of STREAM*s that are read ready.

    for (typename std::vector<STREAM*>::const_iterator i(streams_to_check.begin()); i != streams_to_check.end(); ++i)
    {
        check_these.push_back((*i)->get_read_fd());
        reverse_map[(*i)->get_read_fd()] = *i;
    }

    std::vector<int> readies = AreDescriptorsReadReady(timeout_milliseconds, check_these);

    std::vector<STREAM*> results;

    results.clear();

    for (std::vector<int>::const_iterator i(readies.begin()); i != readies.end(); ++i)
        results.push_back(reverse_map[*i]);

    return results;
}

template<typename STREAM, template<typename > class STL_CONTAINER> std::vector<STREAM*> AreReadReady(const unsigned timeout_milliseconds, STL_CONTAINER<STREAM*>& streams_to_check)
{
    std::vector<int> check_these; // We will need to get the actual descriptors of all the Streams into a vector.
    std::map<int, STREAM*> reverse_map; // used later to quickly build a vector of STREAM*s that are read ready.

    for (typename STL_CONTAINER<STREAM>::const_iterator i(streams_to_check.begin()); i != streams_to_check.end(); ++i)
    {
        check_these.push_back((*i)->get_read_fd());
        reverse_map[(*i)->get_read_fd()] = &(*i);
    }

    std::vector<int> readies = AreDescriptorsReadReady(timeout_milliseconds, check_these);

    std::vector<STREAM*> results;

    for (std::vector<int>::const_iterator i(readies.begin()); i != readies.end(); ++i)
        results.push_back(reverse_map[*i]);

    return results;
}

template<typename STREAM> std::vector<STREAM*> AreWriteReady(const unsigned timeout_milliseconds, std::vector<STREAM*>& streams_to_check)
{
    std::vector<int> check_these; // We will need to get the actual descriptors of all the Streams into a vector.
    std::map<int, STREAM*> reverse_map; // used later to quickly build a vector of STREAM*s that are read ready.
    for (typename std::vector<STREAM*>::const_iterator i(streams_to_check.begin()); i != streams_to_check.end(); ++i)
    {
        check_these.push_back((*i)->get_write_fd());
        reverse_map[(*i)->get_write_fd()] = *i;
    }

    std::vector<int> readies = AreDescriptorsWriteReady(timeout_milliseconds, check_these);

    std::vector<STREAM*> results;

    for (std::vector<int>::const_iterator i(readies.begin()); i != readies.end(); ++i)
        results.push_back(reverse_map[*i]);

    return results;
}

template<typename STREAM, template<typename > class STL_CONTAINER> std::vector<
        STREAM*> AreWriteReady(const unsigned timeout_milliseconds,
        STL_CONTAINER<STREAM>& streams_to_check)
{
    std::vector<int> check_these; // We will need to get the actual descriptors of all the Streams into a vector.
    std::map<int, STREAM*> reverse_map; // used later to quickly build a vector of STREAM*s that are read ready.

    for (typename STL_CONTAINER<STREAM>::const_iterator i(
            streams_to_check.begin()); i != streams_to_check.end(); ++i)
    {
        if ((*i)->eof()) // skip eof descriptors
            continue;

        check_these.push_back(i->get_descriptor());
        reverse_map[i->get_descriptor()] = &(*i);
    }

    std::vector<int> readies = AreDescriptorsWriteReady(timeout_milliseconds,
            check_these);

    std::vector<STREAM*> results;

    for (std::vector<int>::const_iterator i(readies.begin()); i
            != readies.end(); ++i)
        results.push_back(reverse_map[*i]);

    return results;
}

/* Does select return read ready OR is there still data in the local buffer? */
template<typename STREAM, template<typename > class STL_CONTAINER> std::vector<
        STREAM*> HaveData(const unsigned timeout_milliseconds, STL_CONTAINER<
        STREAM*>& streams_to_check)
{
    // First check if any buffered data still exists. Clean it out first;
    std::vector<STREAM*> results;

    for (typename STL_CONTAINER<STREAM*>::iterator i(streams_to_check.begin()); i
            != streams_to_check.end(); ++i)
    {
        if ((*i)->eof()) // skip eof descriptors
            continue;

        if ((*i)->hasBuffered()) // check to see if any buffered data has already been read.
            results.push_back(*i); // we are read ready
        else
        {
            (*i)->fillBuffer(); // attempt to read some data (non-blocking).

            if ((*i)->hasBuffered()) // check to see if any buffered data has already been read.
                results.push_back(*i);
        }
    }

    // if we got any, return this list right now. Don't bother with select on the following.
    if (not results.empty())
        return results;

    std::vector<int> check_these; // We will need to get the actual descriptors of all the Streams into a vector.
    std::map<int, STREAM*> reverse_map; // used later to quickly build a vector of STREAM*s that are read ready.
    for (typename STL_CONTAINER<STREAM*>::iterator i(streams_to_check.begin()); i
            != streams_to_check.end(); ++i)
    {
        // If this fails to compile, your STREAM class doesn't have this. This function only works in the FileDescriptorStream hierarchy
        check_these.push_back((*i)->get_descriptor());
        reverse_map[(*i)->get_descriptor()] = *i;
    }

    std::vector<int> readies = AreDescriptorsReadReady(timeout_milliseconds,
            check_these);

    for (std::vector<int>::const_iterator i(readies.begin()); i
            != readies.end(); ++i)
        results.push_back(reverse_map[*i]);

    return results;
}

/* Does select return read ready OR is there still data in the local buffer? */
template<typename STREAM, template<typename > class STL_CONTAINER> std::vector<
        STREAM*> HaveData(const unsigned timeout_milliseconds, STL_CONTAINER<
        STREAM>& streams_to_check)
{
    // First check if any buffered data still exists. Clean it out first;
    std::vector<STREAM*> pointers = GetMemberPointers(streams_to_check);
    return HaveData(timeout_milliseconds, pointers);
}

/**
 This class holds all the common functionality that file descriptor based classes have. Files, pipes, sockets, fifos, etc. They have so much in common
 it helps to put that common functionality in a single class. In addition to implementing the read and write functions that all Streams need, this one
 also implements ::select in terms of "FileDescriptorStream*" instead of file descriptors (ints). If you are using Stream objects, you shouldn't be
 thinking in terms of file descriptors.

 This class is rather brain dead. It only supports functionality that is broadly associated with file descriptors. Your in your subclasses, your
 "open" function should call set_descriptor() when you get back a file descriptor, socket, pipe of whatever.
 */


class FileDescriptorStream: public Stream
{
    class FileDescriptor
    {
    public:
	int _write_descriptor;
	int _read_descriptor; // Holds the actual linux file descriptor, socket, pipe, fifo handle, whatever.
	unsigned long long _written;
	unsigned long long _read;
	bool _eof;

	FileDescriptor()
	    : _write_descriptor(-2), _read_descriptor(-2), _written(0), _read(0), _eof(false)
	{
	}

	void Close()
	{
	    // don't close standard handles
	    if (_read_descriptor == 0 or _write_descriptor <= 2)
		return;

	    ::close(_read_descriptor);
	    ::close(_write_descriptor);
	    _write_descriptor = -2;
	    _read_descriptor = -2;
	}

	virtual ~FileDescriptor()
	{
	    Close();
	}
    };

    boost::shared_ptr<FileDescriptor> Fd; // Holds the actual linux file descriptor, socket, pipe, fifo handle, whatever.

public:
    void setDefaults();

    int get_read_fd() const
    {
	return Fd->_read_descriptor;
    }

    int get_write_fd() const
    {
	return Fd->_write_descriptor;
    }

    void set_read_fd(int fd)
    {
	Fd->_read_descriptor = fd;
    }

    void set_write_fd(int fd)
    {
	Fd->_write_descriptor = fd;
    }

    void set_fd_eof(bool eof)
    {
	Fd->_eof = eof;
    }

    bool get_fd_eof() const
    {
	return Fd->_eof;
    }

    void set_descriptors(const int read_descriptor, const int write_descriptor);

    virtual const unsigned long long& get_written() const;

    virtual const unsigned long long& get_read() const;

    virtual void increment_written(int amount);

    virtual void increment_read(int amount);

    virtual bool eof();

    /**
     Defaults to -2, invalid but harmless unless actually used. But if -1 is passed, throws an exception. This is to ensure errors are caught if
     stuff like this is done: FileDescriptorStream file(::open("myfile.txt", O_RDWR));
     */
    FileDescriptorStream(const char* resource, const char* options, int read_descriptor = -2, int write_descriptor = -2);

    virtual size_t read(const size_t max_read, char* destination);

    virtual size_t write(const size_t amount, const char* const source);

    virtual bool isReadReady(const unsigned timeout_milliseconds = 0);

    virtual bool isWriteReady(const unsigned timeout_milliseconds = 0);

    virtual FileDescriptorStream* CopyNew() const;

    Text readToDelimiterStringWithTimeout(const char* delimiter, const Stream::OPTIONS options, long timeout_millisecs);

    Text readToDelimiterStrings(const DelimiterList& delimiters, const Stream::OPTIONS options);

    virtual void close();

    virtual ~FileDescriptorStream();

    // conveniences function to help subclasses turn strings into options
    static int parseIntoStandardOptionsBitmask(const char* textual_options);

    static MiniConfig parseTextualOptions(const char* textual_options);

};
