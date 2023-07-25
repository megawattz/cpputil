/*
Copyright 2009 by Walt Howard
$Id: IpPortAddress.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <StlHelpers.h>

class IpPortAddress
{
    sockaddr_in _address;
public:

    IpPortAddress(const sockaddr& generic_address)
    {
        ::memcpy(&_address, &generic_address, sizeof(_address));
    }

    IpPortAddress(const sockaddr_in& ipaddress) :
        _address(ipaddress)
    {
    }

    IpPortAddress(const char* address_and_port = "0.0.0.0:0")
    {
        _address.sin_family = AF_INET;

        if (address_and_port == NULL)
        {
            _address.sin_addr.s_addr = 0;
            _address.sin_port = 0;
            return;
        }

        char host[128] = "0.0.0.0";
        char service[64] = "0";

        // break the host/address name from the service/port number/name
        if (2 != sscanf(address_and_port, " %127[^:]:%63s", host, service))
            throw Exception(LOCATION, , "Unrecognized address format (%s), need host AND port, example: 127.0.0.1:80, yahoo.com:80, email.com:25, 202.134.33.1:21",
						   address_and_port).c_str());

        // if I can scan in 4 groups of digits separated by period, I have an ip address in string form
        int junk;
        int octets = sscanf(host, " %d.%d.%d.%d", &junk, &junk, &junk, &junk);
        if (4 == octets)
        {
            THROW_ON_FALSE(inet_aton(host, &_address.sin_addr));
            //_address.sin_addr.s_addr = htonl(_address.sin_addr.s_addr);
        }
        else
        {
            // Otherwise I have a hostname which I need to look up.
            hostent* ipaddr = gethostbyname(host);

            if (not ipaddr)
                throw Exception(LOCATION, , "Host not found: %s", host).c_str(), errno);

            _address.sin_addr.s_addr
                    = *reinterpret_cast<unsigned long*> (ipaddr->h_addr);
        }

        short int port = atoi(service);

        // if my service starts with a digit
        if (strchr("0123456789", *service))
        {
            _address.sin_port = htons(port);
        }
        else
        {
            // otherwise its a service name like http or rexec or rsh and i need to look up the port
            servent* serv = getservbyname(service, "tcp");
            if (not serv)
                throw Exception(LOCATION, , "Unrecognized tcp service: %s", service).c_str());
            _address.sin_port = serv->s_port;
        }
    }

    operator const sockaddr_in&() const
    {
        return _address;
    }

    operator sockaddr() const
    {
        sockaddr addr;
        ::memcpy(&addr, &_address, sizeof(addr));
        return addr;
    }

    operator const sockaddr_in*() const
    {
        return &_address;
    }

    operator sockaddr*()
    {
        return reinterpret_cast<sockaddr*> (&_address);
    }

    operator Text() const
    {
        return StringPrintf(0, "%s:%d", inet_ntoa(_address.sin_addr), ntohs(
                _address.sin_port));
    }
};
