#pragma once

#include <TcpServiceStream.h>
#include <Text.h>
#include <StlHelpers.h>
#include <Enhanced.h>
#include <map>
#include <StreamFactory.h>

class HttpService
{
    Text Request;
    Text Document;
    int  ResponseCode;

    typedef Enhanced<std::map<Text, Text>> TEXTMAP;

    TEXTMAP Queries;
    TEXTMAP RequestHeaders;
    TEXTMAP ResponseHeaders;

    bool ResponseHeadersWritten;

    // Get the HTTP headers and other info.
    void GetHttp();

    size_t prewrite(const Text& str);

    SocketStreamPtr ServiceStream;

public:
    GETSET(Text, Request);
    GETSET(Text, Document);
    GETSET(int, ResponseCode);
    GETSET_NONCONST(TEXTMAP, Queries);
    GETSET_NONCONST(TEXTMAP, RequestHeaders);
    GETSET_NONCONST(TEXTMAP, ResponseHeaders);

    HttpService(SocketStream* stream, const char* address = 0, const char* options = 0);

    void open(SocketStream* stream = 0, const char* address = 0, const char* options = 0);

    size_t write(const size_t amount, const char* const source);

    size_t writeString(const Text& str);

    HttpService* accept();

    int Authenticate();

    SocketStream* getServiceStream()
    {
	return ServiceStream.get();
    }
};
