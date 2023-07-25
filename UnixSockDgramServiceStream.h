/*
Copyright 2009 by Walt Howard
$Id: UnixSockDgramServiceStream.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <UnixSockAddress.h>
#include <SocketStream.h>

class UnixSockDgramServiceStream: public SocketStream
{
    UnixSockAddress LastPeer;

public:
    GETSET(UnixSockAddress, LastPeer);

    UnixSockDgramServiceStream(const char* address_as_file_name = NULL, const char* options = NULL);

    UnixSockDgramServiceStream(const UnixSockAddress& address, const char* options = NULL);

    virtual size_t read(const size_t max_read, char* destination);

    virtual size_t write(const size_t amount, const char* const source);

    virtual void open(const char* address_as_file_name = NULL, const char* options = NULL);

    virtual SocketAddress* AddressFromString(const char* address_in_string_form) const
    {
	return new UnixSockAddress(address_in_string_form);
    }

    virtual UnixSockDgramServiceStream* accept()
    {
	throw Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "UnixSockDgramServiceStream cannot call accept()");
    }

    virtual Text PeerAddress() const;

    virtual Text LocalAddress() const;
};
