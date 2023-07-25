#include <UnixSockDgramServiceStream.h>

UnixSockDgramServiceStream::UnixSockDgramServiceStream(const char* address, const char* options)
    : SocketStream(address, options)
{
}

UnixSockDgramServiceStream::UnixSockDgramServiceStream(const UnixSockAddress& address, const char* options)
    : SocketStream(address.asString().c_str(), options)
{
}

size_t UnixSockDgramServiceStream::read(const size_t max_read, char* destination)
{
    socklen_t len(sizeof(sockaddr));
    ssize_t rval =  recvfrom(get_read_fd(), destination, max_read, 0, LastPeer, &len);

    if (rval == -1) // if the standard C library returns an error
    {
        // if would block, just return 0 (non-blocking socket). Means "no data here at this time"
        // connection refused on a udp socket is not reliable "which remote address said this? ignore it."
	if (errno == ECONNREFUSED)
	{
	    // throw exception but don't set eof. In a UDP context this doesn't mean the connection failed it means
	    // the peer is gone. Since UDP can have multiple peers, this isn't fatal.
	    throw(Exception(LOCATION, "Remote listener probably gone: %s", getLastPeer().asString().c_str()));
	}

	if (errno != EAGAIN)
	{
	    // This is a real error of some kind
	    set_fd_eof(true);
	    throw(Exception(LOCATION, "UnixSockDgramServiceStream::read error"));
	}

	return 0; // EAGAIN is not an error condition. It's expected in many circumstances (like reading a socket) just return 0 meaning "no data here boss"
    }

    increment_read(rval);

    return rval;
}

size_t UnixSockDgramServiceStream::write(const size_t amount, const char* const source)
{
    ssize_t rval = sendto(get_write_fd(), source, amount, 0, LastPeer, sizeof(sockaddr));

    if (improbable(rval == -1))
    {
	if (improbable(errno != EAGAIN))
	{
	    set_fd_eof(true);
	    throw(Exception(LOCATION, "Error writing file descriptor %d to %s", get_write_fd(), get_resource().c_str()));
	}
	else
	    return 0;
    }

    increment_written(rval);
    return rval;
}

void UnixSockDgramServiceStream::open(const char* address, const char* options)
{
    if (address)
	set_resource(address);

    if (options)
	set_options(options);

    UnixSockAddress bind_address(get_resource().c_str());

    set_resource(bind_address.asString().c_str());

    int sock = socket(PF_UNIX, SOCK_DGRAM, 0);
    if (sock == -1)
	throw(Exception(LOCATION, "socket(PF_UNIX, SOCK_STREAM, 0) failed"));

    set_descriptors(sock, sock);

    // Unix sockets use a file as their stream. Delete it if it already exists.
    unlink(get_resource().c_str());

    if (-1 == ::bind(get_read_fd(), bind_address, sizeof(sockaddr)))
	throw(Exception(LOCATION, "::bind %s", address));

    set_resource(bind_address);
}

Text UnixSockDgramServiceStream::PeerAddress() const
{
    return LastPeer;
}

Text UnixSockDgramServiceStream::LocalAddress() const
{
    sockaddr address;
    socklen_t length(sizeof(address));
    THROW_ON_ERROR(::getsockname(get_read_fd(), &address, &length));
    return UnixSockAddress(address).asString();
}
