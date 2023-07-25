#pragma once

#include <Stream.h>

class NullStream: public Stream
{
    unsigned long long written_count;
    unsigned long long read_count;

public:
    NullStream(const char* name = "Null", const char* options = "");
    virtual size_t read(const size_t max_read, char* destination);
    virtual size_t write(const size_t amount, const char* const source);
    virtual const unsigned long long& get_written() const;
    virtual const unsigned long long& get_read() const;
    virtual void open(const char* = NULL, const char* = NULL);
    virtual NullStream* accept();
    virtual void close();
    virtual bool eof();
    virtual void flush();
    virtual bool isWriteReady(const unsigned timeout_milliseconds = 0);
    virtual bool isReadReady(const unsigned timeout_milliseconds = 0);
    virtual NullStream* CopyNew() const;
};
