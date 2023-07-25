/*
Copyright 2009 by Walt Howard
$Id: FileDescriptorStream.cc 2428 2012-08-14 15:33:13Z whoward $
*/

#include <sys/time.h>
#include <FileDescriptorStream.h>
#include <Enhanced.h>
#include <MiniConfig.h>
#include <Misc.h>

/**
 Use select() on an std::vector of file descriptors.
 */
std::vector<int> AreDescriptorsReadReady(const unsigned timeout_milliseconds, const std::vector<int>& fds_to_check)
{
    for (;;)
    {
	std::vector<int> readies; // will contain descriptors that are readable

	fd_set read_set;
	FD_ZERO(&read_set);

	int highest_descriptor(-1);

	for (std::vector<int>::const_iterator fd(fds_to_check.begin()); fd != fds_to_check.end(); ++fd)
        {
	    if (*fd < 0)
		throw Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "AreDescriptorsReadReady: Invalid file descriptor:%d", *fd);

	    FD_SET(*fd, &read_set);
            highest_descriptor = highest_descriptor > *fd ? highest_descriptor : *fd;
        }

        // Convert milliseconds into seconds and remaining micro seconds.
        struct timeval timeout = { timeout_milliseconds / 1000, (timeout_milliseconds % 1000) * 1000 };

        switch (::select(highest_descriptor + 1, &read_set, NULL, NULL, &timeout))
        {
	case -1:
	    if (errno != EINTR)
		throw(Exception(LOCATION, "%s", FullErrorInfo().c_str()));
	    break; // try again

	case 0:
	    return readies; // readies of course is empty here. We're returning an empty vector.

	default:
	{
	    // If some of the descriptors are read ready, fill the return vector with them.
	    for (std::vector<int>::const_iterator fd(fds_to_check.begin()); fd != fds_to_check.end(); ++fd)
	    {
		if (FD_ISSET(*fd, &read_set))
		    readies.push_back(*fd);
	    }

	    return readies;
	}
	}
    }
}

std::vector<int> AreDescriptorsWriteReady(const unsigned timeout_milliseconds, const std::vector<int>& fds_to_check)
{
    for (;;)
    {
        std::vector<int> readies; // will contain descriptors that are readable

        fd_set write_set;
        FD_ZERO(&write_set);

        int highest_descriptor(-1);

        for (std::vector<int>::const_iterator fd(fds_to_check.begin()); fd != fds_to_check.end(); ++fd)
        {
	    if (*fd < 0)
		throw Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "AreDescriptorsWriteRead: Invalid file descriptor:%d, did you not call open()?", *fd);

	    FD_SET(*fd, &write_set);
	    highest_descriptor = highest_descriptor > *fd ? highest_descriptor : *fd;
        }

        // Convert milliseconds into seconds and remaining micro seconds.
        struct timeval timeout = { timeout_milliseconds / 1000, (timeout_milliseconds % 1000) * 1000 };

        switch (::select(highest_descriptor + 1, NULL, &write_set, NULL, &timeout))
        {
	case -1:
	    if (errno != EINTR)
		throw(Exception(LOCATION, "%s", FullErrorInfo().c_str()));
	    break; // try again

	case 0:
	    return readies; // readies of course is empty here. We're returning an empty vector.

	default:
	{
	    // If some of the descriptors are read ready, fill the return vector with them.
	    for (std::vector<int>::const_iterator fd(fds_to_check.begin()); fd != fds_to_check.end(); ++fd)
	    {
		if (FD_ISSET(*fd, &write_set))
		    readies.push_back(*fd);
	    }
	    return readies;
	}
        }
    }
}

size_t FileDescriptorStream::read(const size_t max_read, char* destination)
{
    /**
       I'm in disagreement with the standard library here. Since this descriptor is non-blocking, reading it and getting nothing is ok. It's expected
       under many circumstances and is not an error. The standard library returns an error if no data is available and a 0 if end of file. This is
       back asswards for non-blocking sockets.
    */

    if (max_read == 0)
        return 0;

    if (improbable(get_read_fd() == -2))
	open();

    int rval = ::read(get_read_fd(), destination, max_read);

    if (rval == 0)
    {
        set_fd_eof(true);
        return 0;
    }

    if (rval == -1) // if the standard C library returns an error
    {
        if (errno != EAGAIN) // and it's not "no data currently available try again" then it really is an error
        {
	    set_fd_eof(true);
            throw(Exception(LOCATION, "FileDescriptorStream::read error:"));
        }
        else
            return 0; // EAGAIN is not an error condition. It's expected in many circumstances (like reading a socket) just return 0 meaning "no data here boss"
    }

    increment_read(rval);
    return rval;
}

size_t FileDescriptorStream::write(const size_t amount,
				   const char* const source)
{
    if (improbable(get_write_fd() == -2))
	open();

    int rval = ::write(get_write_fd(), source, amount);
    if (improbable(rval == -1))
    {
        if (improbable(errno != EAGAIN))
        {
            set_fd_eof(true);
            throw(Exception(LOCATION, "Error writing file descriptor %d to %s", get_write_fd(), get_resource().c_str()));
        }
        else
            return 0;
    }

    increment_written(rval);
    return rval;
}

Text FileDescriptorStream::readToDelimiterStringWithTimeout(
    const char* delimiter, const Stream::OPTIONS options,
    long timeout_millisecs)
{
    Text rval = readToDelimiterString(delimiter, options);
    if (not rval.empty())
        return rval;

    timeval start;
    gettimeofday(&start, NULL);

    do
    {
        isReadReady(timeout_millisecs); // wait here for new data.

        Text rval = readToDelimiterString(delimiter, options);
        if (not rval.empty())
            return rval;

        // if we got data, but not enough to fill our request, wait for any remaining part of the timeout

        timeval now;
        gettimeofday(&now, NULL);

        timeout_millisecs -= (((now.tv_sec - start.tv_sec) * 1000000)
			      + (now.tv_usec - start.tv_usec)) / 1000;
    } while (timeout_millisecs > 0);

    throw(Exception(LOCATION, "Timeout reading to delimiter"));
}

bool FileDescriptorStream::isReadReady(const unsigned timeout_milliseconds)
{
    if (eof())
	throw Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "isReadReady() called on eof Stream");
    std::vector<FileDescriptorStream*> streams;
    streams.push_back(this);
    return hasBuffered() or not AreReadReady(timeout_milliseconds, streams).empty();
}

bool FileDescriptorStream::isWriteReady(const unsigned timeout_milliseconds)
{
    if (eof())
	throw Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "isWriteReady() called on eof Stream");
    std::vector<FileDescriptorStream*> streams;
    streams.push_back(this);
    return not AreWriteReady(timeout_milliseconds, streams).empty();
}

bool FileDescriptorStream::eof()
{
    return get_fd_eof() and not hasBuffered();
}

Text FileDescriptorStream::readToDelimiterStrings(const DelimiterList& delimiters, const Stream::OPTIONS options)
{
    Text rval = Stream::readToDelimiterStrings(delimiters, options);

    if (rval.empty() and get_fd_eof())
	return readString(0x7ffffff); // read remainder of buffer if eof() was reached.
    else
	return rval;
}

int FileDescriptorStream::parseIntoStandardOptionsBitmask(const char* options)
{
    int option_bitmask(0);

    char* textual_options = strdupa(options);

    UpperCase(textual_options);

    if (strstr(textual_options, "ACCMODE"))
        option_bitmask |= O_ACCMODE;

    if (strstr(textual_options, "WRONLY") or strstr(textual_options, "WRITE"))
        option_bitmask |= (O_WRONLY | O_CREAT);

    if (strstr(textual_options, "RDONLY") or strstr(textual_options, "READ"))
        option_bitmask |= O_RDONLY;

    if (option_bitmask & O_WRONLY && option_bitmask & O_RDONLY)
        option_bitmask |= O_RDWR;

    if (strstr(textual_options, "CREATE"))
        option_bitmask |= O_CREAT;

    if (strstr(textual_options, "CREAT"))
        option_bitmask |= O_CREAT;

    if (strstr(textual_options, "EXCL"))
        option_bitmask |= O_EXCL;

    if (strstr(textual_options, "NOCTTY"))
        option_bitmask |= O_NOCTTY;

    if (strstr(textual_options, "TRUNC") or strstr(textual_options, "TRUNCATE"))
        option_bitmask |= O_TRUNC;

    if (strstr(textual_options, "APPEND"))
        option_bitmask |= O_APPEND;

    if (strstr(textual_options, "NONBLOCK"))
        option_bitmask |= O_NONBLOCK;

    if (strstr(textual_options, "DELAY"))
        option_bitmask |= O_NDELAY;

    if (strstr(textual_options, "FASYNC"))
        option_bitmask |= FASYNC;

    if (strstr(textual_options, "DIRECT"))
        option_bitmask |= O_DIRECT;

    if (strstr(textual_options, "LARGEFILE"))
        option_bitmask |= O_LARGEFILE;

    if (strstr(textual_options, "DIRECTORY"))
        option_bitmask |= O_DIRECTORY;

    if (strstr(textual_options, "NOFOLLOW"))
        option_bitmask |= O_NOFOLLOW;

    return option_bitmask;
}

void FileDescriptorStream::setDefaults()
{
    if (get_read_fd() > -1)
	if (-1 == ::fcntl(get_read_fd(), F_SETFL, O_NONBLOCK))
	    throw(Exception(LOCATION, "set nonblocking on read handle:%d", get_read_fd()));

    if ( get_write_fd() > -1)
	if (-1 == ::fcntl(get_write_fd(), F_SETFL, O_NONBLOCK))
	    throw(Exception(LOCATION, "set nonblocking on write handle:%d", get_write_fd()));
}

void FileDescriptorStream::set_descriptors(const int read_descriptor, const int write_descriptor)
{
    if (get_read_fd() > -1)
        throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Stream already attached"));

    set_read_fd(read_descriptor);

    if (get_write_fd() > -1)
        throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Stream already attached"));

    set_write_fd(write_descriptor);

    setDefaults();

    Fd->_written = 0;
    Fd->_read = 0;

    set_fd_eof(false);
}

const unsigned long long& FileDescriptorStream::get_written() const
{
    return Fd->_written;
}

const unsigned long long& FileDescriptorStream::get_read() const
{
    return Fd->_read;
}

inline void FileDescriptorStream::increment_written(int amount)
{
    Fd->_written += amount;
}

inline void FileDescriptorStream::increment_read(int amount)
{
    Fd->_read += amount;
}

/**
    Defaults to -2, invalid but harmless unless actually used. But if -1 is passed, throws an exception. This is to ensure errors are caught if
    stuff like this is done: FileDescriptorStream file(::open("myfile.txt", O_RDWR));
*/
FileDescriptorStream::FileDescriptorStream(const char* resource, const char* options, int read_descriptor, int write_descriptor)
    : Stream(NO_NULL_STR(resource), NO_NULL_STR(options)), Fd(new FileDescriptor())
{
    THROW_ON_ERROR(read_descriptor);
    THROW_ON_ERROR(write_descriptor);

    set_descriptors(read_descriptor, write_descriptor);
}

void FileDescriptorStream::close()
{
    // don't actually close the descriptor if other people are using it.
    if (Fd.use_count() == 1)
	Fd->Close();
}

FileDescriptorStream* FileDescriptorStream::CopyNew() const
{
    FileDescriptorStream* temp = new FileDescriptorStream(get_resource().c_str(), get_option_string().c_str());
    return temp;
}

FileDescriptorStream::~FileDescriptorStream()
{
    close();
}

MiniConfig FileDescriptorStream::parseTextualOptions(const char* textual_options)
{
    MiniConfig options;
    options.loadFromNameValuePairs(textual_options);
    return options;
}
