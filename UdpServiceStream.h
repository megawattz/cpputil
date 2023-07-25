/*
Copyright 2009 by Walt Howard
$Id: UdpServiceStream.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <IpSocketAddress.h>
#include <TcpServiceStream.h>

class UdpServiceStream: public TcpServiceStream
{
    IpSocketAddress LastPeer;

public:

    GETSET(IpSocketAddress, LastPeer);

    UdpServiceStream(const char* address_including_socket = NULL, const char* options = NULL);

    UdpServiceStream(const IpSocketAddress& address, const char* options = NULL);

    virtual size_t read(const size_t max_read, char* destination);

    virtual size_t write(const size_t amount, const char* const source);

    virtual void open(const char* address_including_socket = NULL, const char* options = NULL);

    virtual bool isWriteReady(const unsigned timeout_milliseconds);

    virtual UdpServiceStream* accept()
    {
	throw Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "UdpServiceStream cannot call accept()");
    }

    virtual Text PeerAddress() const;
};
