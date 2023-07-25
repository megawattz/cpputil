/*
Copyright 2009 by Walt Howard
$Id: StringAsStream.cc 2428 2012-08-14 15:33:13Z whoward $
*/

#include <StringAsStream.h>
#include <Misc.h>

StringAsStream::StringAsStream(std::string& target)
    : Stream("StringVariable"), _data(target), is_eof(false)
{
    open();
}


StringAsStream::StringAsStream(const char* data, const char* options)
    : Stream("StringVariable", options), local_data(data), _data(local_data), is_eof(false)
{
    open();
}

void StringAsStream::open(const char* resource, const char* options)
{
    _position = _data.begin();
    _written = 0;
    _read = 0;
    is_eof = false;
}


bool StringAsStream::eof()
{
    return false;
}


void StringAsStream::close()
{
    _position = _data.end();
}


size_t StringAsStream::write(const size_t amount, const char* const source)
{
    _data.append(source);
    _written += amount;
    return amount;
}


bool StringAsStream::isWriteReady(const unsigned timeout_milliseconds)
{
    return true;
}


bool StringAsStream::isReadReady(const unsigned timeout_milliseconds)
{
    return true;
}


const unsigned long long& StringAsStream::get_written() const
{
    return _written;
}


const unsigned long long& StringAsStream::get_read() const
{
    return _read;
}


size_t StringAsStream::read(const size_t max_read, char* destination)
{
    size_t length = std::min(static_cast<size_t>(_data.end() - _position), max_read);
    ::memcpy(destination, &*_position, length);
    _position += length;
    _read += length;
    if (length == 0)
	is_eof = true;
    return length;
}

StringAsStream* StringAsStream::CopyNew() const
{
    StringAsStream* temp = new StringAsStream(get_resource().c_str(), get_option_string().c_str());
    return temp;
}
