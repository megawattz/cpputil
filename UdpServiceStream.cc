#include <UdpServiceStream.h>

UdpServiceStream::UdpServiceStream(const char* address_including_socket, const char* options)
    : TcpServiceStream(address_including_socket, options)
{
}

UdpServiceStream::UdpServiceStream(const IpSocketAddress& address, const char* options)
    : TcpServiceStream(address.asString().c_str(), options)
{
}

size_t UdpServiceStream::read(const size_t max_read, char* destination)
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
	    throw(Exception(LOCATION, "UdbServiceStream::read error"));
	}

	return 0; // EAGAIN is not an error condition. It's expected in many circumstances (like reading a socket) just return 0 meaning "no data here boss"
    }

    increment_read(rval);

    return rval;
}

size_t UdpServiceStream::write(const size_t amount, const char* const source)
{
    ssize_t rval = sendto(get_write_fd(), source, amount, 0, LastPeer, sizeof(sockaddr));

    if (improbable(rval == -1))
    {
	if (improbable(errno != EAGAIN))
	{
	    throw(Exception(LOCATION, "Error writing file descriptor %d to %s", get_write_fd(), get_resource().c_str()));
	}
	else
	    return 0;
    }

    increment_written(rval);
    return rval;
}

void UdpServiceStream::open(const char* address_including_socket, const char* options)
{
    if (address_including_socket)
	set_resource(address_including_socket);

    if (options)
	set_options(options);

    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
	throw(Exception(LOCATION, "socket(PF_INET, SOCK_STREAM, 0) failed"));

    set_descriptors(sock, sock);

    SetOptions();

    IpSocketAddress binder(get_resource().c_str());

    if (-1 == ::bind(get_read_fd(), binder, sizeof(sockaddr)))
	throw(Exception(LOCATION, "::bind %s", address_including_socket));
}

bool UdpServiceStream::isWriteReady(const unsigned timeout_milliseconds)
{
    std::vector<FileDescriptorStream*> streams;
    streams.push_back(this);
    return not AreWriteReady(timeout_milliseconds, streams).empty();
}

Text UdpServiceStream::PeerAddress() const
{
    return LastPeer.asString();
}
