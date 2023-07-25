/*
Copyright 2009 by Walt Howard
$Id: UdpClientStream.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <TcpClientStream.h>

class UdpClientStream: public TcpClientStream
{
    IpSocketAddress LastPeer;

public:
    GETSET(IpSocketAddress, LastPeer);

    UdpClientStream(const char* address_including_socket = NULL, const char* options = NULL);

    UdpClientStream(const SocketAddress& address, const char* options = NULL);

    virtual void open(const char* address_including_socket = NULL, const char* options = NULL);

    virtual size_t write(const size_t amount, const char* const source);

    Text PeerAddress() const
    {
	return LastPeer.asString();
    }

    virtual UdpClientStream* accept()
    {
	throw Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "UdpClient cannot call accept()");
    }
};
