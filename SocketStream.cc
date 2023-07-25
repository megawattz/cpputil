/*
Copyright 2009 by Walt Howard
$Id: SocketStream.cc 2428 2012-08-14 15:33:13Z whoward $
*/

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <FileDescriptorStream.h>
#include <SocketAddress.h>
#include <SocketStream.h>

#ifdef __linux__
#include <linux/sockios.h>
#endif

SocketStream::SocketStream(const char* address_including_socket, const char* options) :
    FileDescriptorStream(address_including_socket, options), _connect_timeout(0)
{
    _connect_timeout = atoi(get_options().getValue("timeout").c_str());
}

SocketStream::SocketStream(const char* address, const char* options, int socket) :
    FileDescriptorStream(address, options, socket, socket), _connect_timeout(0)
{
    _connect_timeout = atoi(get_options().getValue("timeout").c_str());
}

int SocketStream::getSocketError()
{
    int errval(0);
    socklen_t length = sizeof(errval);
    THROW_ON_ERROR(getsockopt(get_read_fd(), SOL_SOCKET, SO_ERROR, &errval, &length));
    return errval;
}

void SocketStream::SetOptions()
{
    Text input_buffer_size = get_options().getValue("input-buffer", 0);

    if (not input_buffer_size.empty())
    {
	int bufsize = atoi(input_buffer_size.c_str());

	THROW_ON_ERROR( setsockopt( get_read_fd(), SOL_SOCKET, SO_RCVBUF,
				    (const char*)&bufsize, sizeof( bufsize )));
    }

    Text output_buffer_size = get_options().getValue("output-buffer", 0);

    if (not output_buffer_size.empty())
    {
	int bufsize = atoi(output_buffer_size.c_str());

	THROW_ON_ERROR( setsockopt( get_write_fd(), SOL_SOCKET, SO_SNDBUF,
				    (const char*)&bufsize, sizeof( bufsize )));
    }

    Text broadcast = get_options().getValue("broadcast", 0);

    if (not output_buffer_size.empty())
    {
	int bufsize = atoi(output_buffer_size.c_str());

	THROW_ON_ERROR( setsockopt( get_write_fd(), SOL_SOCKET, SO_BROADCAST,
				    (const char*)&bufsize, sizeof( bufsize )));
    }

    Text nagle_off = get_options().getValue("nagle_off", 0);

    if (nagle_off)
    {
	int yes(1);
	THROW_ON_ERROR(setsockopt(get_write_fd(), IPPROTO_TCP, TCP_NODELAY,
				  &yes, sizeof(yes))); // Nagle off
    }

}

/*
SocketAddress SocketStream::localAddress() const
{
    sockaddr address;
    socklen_t length(sizeof(address));
    THROW_ON_ERROR(::getsockname(get_read_fd(), &address, &length));
    return address;
}

SocketAddress SocketStream::peerAddress() const
{
    sockaddr address;
    socklen_t length(sizeof(address));
    THROW_ON_ERROR(::getpeername(get_read_fd(), &address, &length));
    return address;
}
*/

void SocketStream::close()
{
    // Don't use this at this time. It causes all instances of process sharing the socket to disconnect because it tells the OTHER end to shutdown.
#ifdef __KJHKHlinux__
    int lastcount(0);
    int count(0);

    // if successful at using this ioctl() enter the wait loop
    if (-1 != ioctl(get_write_fd(), SIOCOUTQ, &lastcount))
    {
	for(;;)
	{
	    if (-1 == ioctl(get_write_fd(), SIOCOUTQ, &count))
		break;

	    // no progress, (hopefully both are 0) then give up
	    if (probable(count == lastcount))
		break;

	    usleep(count); // wait as many microseconds as bytes remaining.

	    lastcount = count;
	}
    }
#endif

    FileDescriptorStream::close();
}

SocketStream::~SocketStream()
{
    close();
}
