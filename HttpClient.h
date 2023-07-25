/*
Copyright 2009 by Walt Howard
$Id: HttpClient.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <Stream.h>
#include <StreamFactory.h>

class HttpClient
{
    Text Host;
    Text Path;
    Text QueryString;

    void Parse(const char* url, const char* options);

    SocketStreamPtr ClientStream;

public:
    HttpClient(SocketStream* stream, const char* url, const char* options);

    void open(SocketStream* stream, const char* url, const char* options);
};
