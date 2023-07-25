/*
Copyright 2009 by Walt Howard
$Id: Stream.cc 2428 2012-08-14 15:33:13Z whoward $
*/

#include <Stream.h>
#include <fcntl.h>

Stream::Stream(const char* resource, const char* options, const int size) :
    _options(NO_NULL_STR(options)), _option_string(NO_NULL_STR(options)),_resource(NO_NULL_STR(resource)),
    _buffer(new Buffer(size)), _debug_file(-1)
{
    const char* debug = getenv("STREAM_MONITOR");

    if (debug)
    {
        _debug_file = ::open(StringPrintf(0, "%s-%s.stream_monitor.txt", debug,
					  resource).c_str(), O_CREAT | O_SYNC | O_TRUNC | O_RDWR, 0xfff);

        THROW_ON_ERROR(_debug_file);
    }
}


void Stream::open(const char* resource, const char* options)
{
    if (resource)
	_resource = resource;

    if (options)
    {
	_option_string = options;
	_options.loadFromNameValuePairs(options);
    }
}


void Stream::close()
{
}


Stream* Stream::accept()
{
    return this;
}


size_t Stream::writeAll(const size_t total_amount,  const char* const original_source, int milliseconds_to_wait)
{
    size_t amount(total_amount);
    const char* source(original_source);

    do
    {
	if (milliseconds_to_wait and !isWriteReady(milliseconds_to_wait))
	    throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Failed to write after %d milli-second timeout to %s", milliseconds_to_wait, _resource.c_str()));

	size_t rval = write(amount, source);
        if (probable(rval == amount))
        {
            if (_debug_file > -1)
                THROW_ON_ERROR(::write(_debug_file, original_source, total_amount));
            return total_amount;
        }
        amount -= rval;
        source += rval;
    } while(amount > 0);

    return total_amount;

    //throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Write/Attempt count mismatch. Should never come here"));
}


size_t Stream::Printf(const char* format, ...)
{
    size_t bufsize(4096);

    for (;;)
    {
        char* const output_buffer(reinterpret_cast<char *> (::alloca(bufsize)));
        va_list args;
        va_start(args, format);
        const int written = ::vsnprintf(output_buffer, bufsize, format, args);
        va_end(args);

        // If there was insufficient room, set size to what is needed and try again
        // if vsnprintf fails it returns the number of bytes NEEDED so we can try again and succeed
        if (written >= static_cast<int> (bufsize))
        {
            bufsize = written + 1;
            continue;
        }

        if (written < 0)
            throw(Exception(LOCATION, "Error writing printf style output"));

        writeAll(written, output_buffer);
        return written;
    }
}

size_t Stream::writeString(const Text& write_me)
{
    return writeAll(write_me.size(), write_me.c_str());
}


namespace {

inline char* memsrch(void* memory, const char* str, size_t length)
{
    int checklen = strlen(str);
    const char* end = reinterpret_cast<char*>(memory) + length - checklen + 1;
    for(char* s = reinterpret_cast<char*>(memory); s < end; ++s)
    {
	if (probable(*s != *str)) // not even a beginning of a match, go to next
	    continue;

	// potential match? check it.
	if (probable(not strncmp(str, s, checklen)))
	    return reinterpret_cast<char*>(s);
    }

    return 0;
}

}


const char* Stream::FirstDelimiter(const Stream::DelimiterList& delimiters)
{
    const char* first_delimiter(NULL);

    for (Stream::DelimiterList::const_iterator delimiter(delimiters.begin()); delimiter
            != delimiters.end(); ++delimiter)
    {
	const char* read_point = _buffer->_read_point;
	const char* insert_point = _buffer->_insert_point;
        const char* next_delimiter = ::memsrch(_buffer->_read_point, *delimiter, _buffer->_insert_point - _buffer->_read_point); // find the delimiter
        if (next_delimiter)
            if (next_delimiter < first_delimiter or not first_delimiter)
                first_delimiter = *delimiter;
    }
    // first element points to WHERE the the delimiter is, second points to the delimiter TEXT that we found. There is a difference.
    // if we are looking for "proto" we find it in "good protoplasm". first points to the "p" in protoplasm, second points to the Text.c_str() "proto"
    return first_delimiter;
}

size_t Stream::fillBuffer()
{
    // move any remaining junk to the beginning of the buffer in prepartion for refilling the buffer
    int length(_buffer->_insert_point - _buffer->_read_point); // how much buffer do we have left unread?
    ::memmove(_buffer->_read_buffer, _buffer->_read_point, length); //move it to the beginning of the buffer space.
    _buffer->_read_point = _buffer->_read_buffer; // also adjust the readpoint
    _buffer->_insert_point = _buffer->_read_point + length; // new data will be inserted right after the old unread text in the buffer
    *_buffer->_insert_point = '\0'; // prevent reading past the insert_point (if not wraparound bugs can occur)

    int read_attempt = _buffer->_end - _buffer->_insert_point - 1;

    int amount_read = this->read(read_attempt, _buffer->_insert_point); // try to completely fill the buffer;

    if (0 == amount_read) // if nothing available to read, return "no string"
        return 0;

    _buffer->_insert_point += amount_read; // adjust insertion point

    *_buffer->_insert_point = '\0'; // prevent reading past the insert_point (if not wraparound bugs can occur)

    return amount_read;
}

size_t Stream::readAll(size_t amount, char* destination)
{
    if (_buffer->_insert_point - _buffer->_read_point < static_cast<int>(amount))
        fillBuffer();

    if (_buffer->_insert_point - _buffer->_read_point < static_cast<int>(amount))
        return 0;

    ::memcpy(destination, _buffer->_read_point, amount);
    _buffer->_read_point += amount;

    return amount;
}

Text Stream::readString(const size_t max_length)
{
    char* buffer = (char*) alloca(max_length + 1);
    char* p = buffer;
    size_t total_read(0);

    while(total_read < max_length and not eof())
    {
	// if there is less than max_length in the buffer, try to get some more.
	if (_buffer->_insert_point - _buffer->_read_point < static_cast<int>(max_length))
	    fillBuffer();

	size_t amount = MIN((_buffer->_insert_point - _buffer->_read_point), (max_length - total_read));

	if (not amount)
	    break;

	::memcpy(p, _buffer->_read_point, amount);

	p += amount;

	total_read += amount;

	_buffer->_read_point += amount;
    }

    return Text(buffer, total_read);
}

size_t Stream::relay(const size_t atmost, Stream* output, const char* delimiter,
		     const Stream::OPTIONS options)
{
    // not enough data to fullfill the "starting" location
    if (_buffer->_insert_point - _buffer->_read_point < static_cast<int>(atmost))
	fillBuffer();

    // if we have anything now, try to read in a delimiter
    if (_buffer->_insert_point - _buffer->_read_point == 0)
	return 0;

    int search_len = MIN(_buffer->_insert_point - _buffer->_read_point, atmost);

    const char* start_search =  _buffer->_read_point + search_len;
    const char* d = delimiter;

    for(const char* s = start_search; s > _buffer->_read_point; --s)
    {
	if (improbable(*s == *d))
	{
	    if (strstr(s, d) == s)
	    {
		int amount = (s - _buffer->_read_point) + strlen(d);

		amount = output->writeAll(amount, _buffer->_read_point);

		_buffer->_read_point += amount;

		return amount;
	    }
	}
    }

    return 0;
}

Text Stream::readToDelimiterStrings(const DelimiterList& delimiters,
        const Stream::OPTIONS options)
{
    const char* first_delimiter = FirstDelimiter(delimiters);

    if (not first_delimiter) // if there is no delimiter in the buffer, read in some new stuff
    {
        if (not fillBuffer())
            return ""; // no new data, return immediately

        // Try again to find a best delimiter
        first_delimiter = FirstDelimiter(delimiters);

        if (not first_delimiter)
	{
	    int length = _buffer->_end - _buffer->_insert_point;
	    if (length < 2) // No delimiter read yet, no more room left in buffer, throw
                throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "No delimiter within entire buffer size %ld", _buffer->get_buffer_size()));
	    return "";
        }
    }

    char* rval_start = _buffer->_read_point;
    _buffer->_read_point = ::memsrch(_buffer->_read_point, first_delimiter, _buffer->_insert_point - _buffer->_read_point);
    _buffer->_read_point += ::strlen(first_delimiter); // skip past the delimiter
    return Text(rval_start, _buffer->_read_point - rval_start - (options
            & Stream::INCLUDE_DELIMITER ? 0 : strlen(first_delimiter)));
}

bool Stream::hasBuffered() const
{
    return _buffer->_insert_point - _buffer->_read_point > 1;
}


size_t Stream::gcount()
{
    size_t temp = _gcount;
    _gcount = 0;
    return temp;
}


bool Stream::isFinite() const
{
    return false;
}


Text Stream::readToDelimiterString(const char* delimiter,
        const Stream::OPTIONS options)
{
    return readToDelimiterStrings(DelimiterList(1, delimiter), options);
}

void Stream::flush()
{
    _buffer->Flush();
}

bool Stream::eof()
{
    try
    {
        fillBuffer();
    } catch (const std::exception& ex)
    {
        // if eof was reached AND we don't have any data in the buffer, we are EOF
        if (_buffer->_insert_point - _buffer->_read_point < 2)
            return true;
    }

    return false;
}

Text Stream::PeerAddress() const
{
    return "";
}

Text Stream::LocalAddress() const
{
    return "";
}

Stream::~Stream()
{
}
