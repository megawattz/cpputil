/*
Copyright 2009 by Walt Howard
$Id: IpSocketAddress.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <SocketAddress.h>
#include <Exception.h>
#include <cstdio>
#include <sys/types.h>
#include <netinet/in.h>
#include <Text.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <Misc.h>

class IpSocketAddress: public SocketAddress
{
    sockaddr_in IpAddress;

public:
    IpSocketAddress(const sockaddr_in& in_address)
	: IpAddress(in_address)
    {
    }

    IpSocketAddress(const sockaddr& sock_address)
    {
	memcpy(&IpAddress, &sock_address, std::min(sizeof(IpAddress), sizeof(sock_address)));
    }

    IpSocketAddress(const char* address_and_socket = "0.0.0.0:0");

    virtual operator sockaddr_in*()
    {
        return &IpAddress;
    }

    virtual operator const sockaddr_in*() const
    {
        return &IpAddress;
    }

    virtual operator sockaddr*()
    {
        return reinterpret_cast<sockaddr*>(&IpAddress);
    }

    virtual operator const sockaddr*() const
    {
        return reinterpret_cast<const sockaddr*>(&IpAddress);
    }

    short getPort() const
    {
	return ntohs(IpAddress.sin_port);
    }

    void setPort(short port)
    {
	IpAddress.sin_port = htons(port);
    }

    virtual operator Text() const
    {
	return StringPrintf(0, "%s:%d", inet_ntoa(IpAddress.sin_addr), ntohs(IpAddress.sin_port));
    }

    virtual Text asString() const
    {
	return operator Text();
    }

    virtual ~IpSocketAddress()
    {
    }
};
