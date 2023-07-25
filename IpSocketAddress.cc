/*
Copyright 2009 by Walt Howard
$Id: IpSocketAddress.cc 2428 2012-08-14 15:33:13Z whoward $
*/

#include "IpSocketAddress.h"

IpSocketAddress::IpSocketAddress(const char* address_and_socket)
{
    unsigned short port;
    char host[128];

    int rval = sscanf(address_and_socket, " %127[^:; ]%*[:; ]%hd", host, &port);

    if (rval < 1)
	throw Exception(LOCATION, StringPrintf(0, "Error parsing ip address: %s", address_and_socket).c_str(), Exception::NO_SYSTEM_ERROR);

    char junk[4];

    rval = sscanf(host, " %3[0-9]%*[.]%3[0-9]%*[.]%3[0-9]%*[.]%3[0-9]", junk, junk, junk, junk);
    if (4 != rval)
    {
	// this is a dns name, not an ip address

	char auxilliary[1024];
	struct hostent host_ent;
	struct hostent* result = &host_ent;
	int error;

	int rval = gethostbyname_r(host, &host_ent, auxilliary, sizeof(auxilliary), &result, &error);

	if (rval or result == 0)
	{
	    if (rval == ERANGE)
		throw Exception(LOCATION, "gethostbyname_r  failed: %d, probably needs more space than 1024 (see this code)", error);

	    throw Exception(LOCATION, "Could not find address for host \"%s\"", host);
	}

	IpAddress.sin_addr.s_addr = *reinterpret_cast<unsigned long*>(host_ent.h_addr);
	IpAddress.sin_port = htons(port);
	IpAddress.sin_family = AF_INET;
    }
    else
    {
	THROW_ON_ERROR(inet_aton(host, &(IpAddress.sin_addr)));
	IpAddress.sin_port = htons(port);
	IpAddress.sin_family = AF_INET;
	memset(IpAddress.sin_zero, '\0', sizeof(IpAddress.sin_zero));
    }
}
