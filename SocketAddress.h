/*
Copyright 2009 by Walt Howard
$Id: SocketAddress.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <Exception.h>
#include <cstdio>
#include <sys/types.h>
#include <netinet/in.h>
#include <Text.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <Misc.h>

class SocketAddress
{
public:
    SocketAddress() {}

    // so you can use an instance of this class directly in socket calls that take
    // sockaddr* arguments.
    virtual operator sockaddr*() = 0;

    // so you can use an instance of this class directly in socket calls that take
    // sockaddr* arguments.
    virtual operator const sockaddr*() const = 0;

    virtual operator Text() const
    {
	return "GenericSockAddr";
    }

    virtual Text asString() const
    {
	return operator Text();
    }

    virtual ~SocketAddress()
    {
    }
};
