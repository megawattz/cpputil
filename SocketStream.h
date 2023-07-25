/*
Copyright 2009 by Walt Howard
$Id: SocketStream.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <FileDescriptorStream.h>
#include <SocketAddress.h>

class SocketStream: public FileDescriptorStream
{
    unsigned _connect_timeout;

public:
    GETSET(unsigned, _connect_timeout);

    SocketStream(const char* address_including_socket = NULL, const char* options = NULL);

    SocketStream(const char* address, const char* options, int socket);

    virtual SocketStream* accept() = 0;

    virtual SocketAddress* AddressFromString(const char* address_in_string_form) const = 0;

    int getSocketError();

    void SetOptions();

    virtual void close();

    virtual ~SocketStream();
};
