#include "NullStream.h"

NullStream::NullStream(const char* name, const char* options)
    : Stream(name, options), written_count(0), read_count(0)
{
}

size_t NullStream::read(const size_t max_read, char* destination)
{
    return 0;
};

size_t NullStream::write(const size_t amount, const char* const source)
{
    written_count += amount; return amount;
}

const unsigned long long& NullStream::get_written() const
{
    return written_count;
}

const unsigned long long& NullStream::get_read() const
{
    return read_count;
}

void NullStream::open(const char* name, const char* options)
{
    Stream::open(name, options);
}

NullStream* NullStream::accept()
{
    return new NullStream;
}

void NullStream::close()
{
}

bool NullStream::eof()
{
    return false;
}

void NullStream::flush()
{
}

bool NullStream::isWriteReady(const unsigned timeout_milliseconds)
{
    return true;
}

bool NullStream::isReadReady(const unsigned timeout_milliseconds)
{
    return false;
}

NullStream* NullStream::CopyNew() const
{
    NullStream* temp = new NullStream(get_resource().c_str(), get_option_string().c_str());
    return temp;
}
