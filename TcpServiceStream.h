/*
Copyright 2009 by Walt Howard
$Id: TcpServiceStream.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <FileDescriptorStream.h>
#include <IpSocketAddress.h>
#include <SocketStream.h>

class TcpServiceStream: public SocketStream
{
protected:
    TcpServiceStream(int socket, const char* address, const char* options = "");

public:
    TcpServiceStream(const char* address_including_port = NULL, const char* options = NULL);

    TcpServiceStream(const SocketAddress& address, const char* options);

    virtual void open(const char* address_including_port = NULL, const char* = NULL);

    virtual TcpServiceStream* accept();

    virtual IpSocketAddress* AddressFromString(const char* address_in_string_form) const
    {
	return new IpSocketAddress(address_in_string_form);
    }

    virtual Text PeerAddress() const;

    virtual Text LocalAddress() const;
};
