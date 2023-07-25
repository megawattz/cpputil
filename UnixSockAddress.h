/*
Copyright 2009 by Walt Howard
$Id: UnixSockAddress.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <SocketAddress.h>
#include <Exception.h>
#include <cstdio>
#include <sys/types.h>
#include <sys/un.h>
#include <Text.h>
#include <Misc.h>

class UnixSockAddress: public SocketAddress
{
    sockaddr_un UnixAddress;

public:
    UnixSockAddress(const sockaddr_un& un_address)
	: UnixAddress(un_address)
    {
    }

    UnixSockAddress(const sockaddr& sock_address)
    {
	memcpy(&UnixAddress, &sock_address, std::min(sizeof(UnixAddress), sizeof(sock_address)));
    }

    UnixSockAddress(const char* address = "unix_sock")
    {
	UnixAddress.sun_family = AF_UNIX;
	strncpy(UnixAddress.sun_path, address, sizeof(UnixAddress.sun_path));
    }

    virtual operator sockaddr_un*()
    {
        return &UnixAddress;
    }

    virtual operator const sockaddr_un*() const
    {
        return &UnixAddress;
    }

    virtual operator sockaddr*()
    {
	return reinterpret_cast<sockaddr*>(&UnixAddress);
    }

    virtual operator const sockaddr*() const
    {
	return reinterpret_cast<const sockaddr*>(&UnixAddress);
    }

    virtual operator Text() const
    {
	return UnixAddress.sun_path;
    }

    virtual Text asString() const
    {
	return operator Text();
    }

    virtual ~UnixSockAddress()
    {
    }
};
