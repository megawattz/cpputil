/*
Copyright 2009 by Walt Howard
$Id: TcpClientStream.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <IpSocketAddress.h>
#include <TcpServiceStream.h>

class TcpClientStream: public TcpServiceStream
{
    IpSocketAddress IpAddress;

public:
    TcpClientStream(const char* address_including_socket = NULL,const char* options = NULL);

    TcpClientStream(const IpSocketAddress& address, const char* options = NULL);

    virtual void open(const char* address_including_socket = NULL, const char* options = NULL);

    virtual TcpClientStream* accept()
    {
	throw Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "TcpClient cannot call accept()");
    }
};
