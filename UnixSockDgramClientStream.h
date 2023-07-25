/*
Copyright 2009 by Walt Howard
$Id: UnixSockDgramClientStream.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <UnixSockDgramServiceStream.h>

class UnixSockDgramClientStream: public UnixSockDgramServiceStream
{
    Text BindFileName; // Bind address is just a file name

public:
    UnixSockDgramClientStream(const char* address_as_file_name = NULL, const char* options = NULL);

    UnixSockDgramClientStream(const UnixSockAddress& address, const char* options = NULL);

    virtual void open(const char* address_including_socket = NULL, const char* options = NULL);

    virtual UnixSockDgramClientStream* accept()
    {
	throw Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "UnixSockDgramClientStream cannot call accept()");
    }

    virtual void close();
};
