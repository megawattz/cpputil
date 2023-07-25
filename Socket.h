/*
Copyright 2009 by Walt Howard
$Id: Socket.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <Exception.h>
#include <Text.h>
#include <Misc.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

class Socket
{
    int fd;

public:
    Socket(const int socket_type, const int protocol)
    {
        THROW_ON_ERROR(fd = socket(PF_INET, socket_type, 0));
        DefaultOptions();
    }

    Socket(int socket) :
        fd(socket)
    {
        DefaultOptions();
    }

    void DefaultOptions()
    {
        int one(1);
        // always set non-blocking and reuse address
        THROW_ON_ERROR(::ioctl(fd, FIONBIO, (char *)&one));
        THROW_ON_ERROR(::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)));
    }

    int Write(const void* buffer, int amount = -1)
    {
        if (amount == -1)
            amount = strlen(reinterpret_cast<const char*> (buffer));

        int rval = ::write(fd, buffer, amount);

        if (rval > 0)
            return rval;

        // NOTE: Different behavior than normal!
        if (rval == EWOULDBLOCK)
            return 0;

        if (rval == -1)
            throw Exception(LOCATION, StringPrintf(0, "Error writing %d bytes to socket %p", amount, this).c_str(), rval);

        return 0;
    }

    void Bind(const sockaddr& address)
    {
        THROW_ON_ERROR(::bind(fd, &address, sizeof(sockaddr)));
    }

    ssize_t Read(void* buffer, const ssize_t amount)
    {
        ssize_t rval = ::read(fd, buffer, amount);

        if (rval > 0)
            return rval;

        // NOTE: Different behavior than normal! With a non-blocking socket, 0 bytes read
        // is an expected condition!
        if (rval == EWOULDBLOCK)
            return 0;

        if (rval == -1)
            throw Exception(LOCATION, StringPrintf(0, "Error reading %ld bytes from socket %p", amount, this).c_str(),
                    rval);

        return -1;
    }

    void ShutdownWrite()
    {
        THROW_ON_ERROR(::shutdown(fd, SHUT_WR));
    }

    void ShutdownRead()
    {
        THROW_ON_ERROR(::shutdown(fd, SHUT_RD));
    }

    void Shutdown()
    {
        THROW_ON_ERROR(::shutdown(fd, SHUT_RDWR));
    }

    void Close()
    {
        THROW_ON_ERROR(::close(fd));
    }

    operator int()
    {
        return fd;
    }

    virtual ~Socket()
    {
        try
        {
            Close();
        } catch (Exception& e)
        {
            // suppress exceptions in destructors
        }
    }
};
