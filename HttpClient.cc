/*
Copyright 2009 by Walt Howard
$Id: HttpClient.cc 2428 2012-08-14 15:33:13Z whoward $
*/

#include <HttpClient.h>
#include <StreamFactory.h>

void HttpClient::Parse(const char* url, const char* options)
{
    if (url)
    {
	char host[128] = "";
	char path[2048] = "/";

	if (const char* p = strchr(url, ':'))
	    url = p + 1;

	if (const char* p = strstr(url, "//"))
	    url = p + 2;

	if (1 > sscanf(url, "%127[^/]%2047s", host, path))
	    throw(Exception(LOCATION, "Invalid HTTP request: %s", url));

	Host = host;
	Path = path;
    }
}


HttpClient::HttpClient(SocketStream* stream, const char* url, const char* options)
    : ClientStream(stream)
{
    Parse(url, options);
}


void HttpClient::open(SocketStream* stream, const char* url, const char* options)
{
    if (stream)
	ClientStream = stream;

    if (url or options)
	Parse(url, options);

    stream->open(Host.c_str(), options);
}
