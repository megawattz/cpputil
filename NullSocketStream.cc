#include <NullSocketStream.h>

NullSocketStream::NullSocketStream(const char* name, const char* options)
    : SocketStream(name, options), written_count(0), read_count(0)
{
}

size_t NullSocketStream::read(const size_t max_read, char* destination)
{
    return 0;
};

size_t NullSocketStream::write(const size_t amount, const char* const source)
{
    written_count += amount; return amount;
}

const unsigned long long& NullSocketStream::get_written() const
{
    return written_count;
}

const unsigned long long& NullSocketStream::get_read() const
{
    return read_count;
}

void NullSocketStream::open(const char* name, const char* options)
{
    Stream::open(name, options);
}

NullSocketStream* NullSocketStream::accept()
{
    return new NullSocketStream;
}

void NullSocketStream::close()
{
}

bool NullSocketStream::eof()
{
    return false;
}

void NullSocketStream::flush()
{
}

bool NullSocketStream::isWriteReady(const unsigned timeout_milliseconds)
{
    return true;
}

bool NullSocketStream::isReadReady(const unsigned timeout_milliseconds)
{
    return true;
}

SocketAddress* NullSocketStream::AddressFromString(const char* address) const
{
    return 0;
}

NullSocketStream* NullSocketStream::CopyNew() const
{
    NullSocketStream* temp = new NullSocketStream(get_resource().c_str(), get_option_string().c_str());
    return temp;
}

NullSocketStream::~NullSocketStream()
{
}
