#include <UdpClientStream.h>

UdpClientStream::UdpClientStream(const char* address_including_socket, const char* options)
    : TcpClientStream(address_including_socket, options)
{
}

UdpClientStream::UdpClientStream(const SocketAddress& address, const char* options)
    : TcpClientStream(address.asString().c_str(), options)
{
}

void UdpClientStream::open(const char* address_including_socket, const char* options)
{
    if (address_including_socket)
	set_resource(IpSocketAddress(address_including_socket));

    IpSocketAddress connect_to(get_resource().c_str());

    set_resource(connect_to.asString().c_str());

    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
	throw(Exception(LOCATION, "socket(PF_INET, SOCK_STREAM, 0) failed"));

    set_descriptors(sock, sock);

    SetOptions();

    setLastPeer(connect_to);

    // udp doesn't really "connect" but this sets a default destination for packets
    // This condition may not be fatal, that is, a server can be brought up AFTER udp sets it default destination
    if (-1 == ::connect(get_read_fd(), connect_to, sizeof(sockaddr)))
	throw(Exception(LOCATION, "::bind %s", get_resource().c_str()));
}

size_t UdpClientStream::write(const size_t amount, const char* const source)
{
    if (improbable(get_write_fd() == -2))
	open();

    ssize_t rval = sendto(get_write_fd(), source, amount, 0, LastPeer, sizeof(sockaddr));

    if (improbable(rval == -1))
    {
	if (improbable(errno != EAGAIN))
	{
	    throw(Exception(LOCATION, "Non fatal error writing udp %d to %s", get_write_fd(), get_resource().c_str()));
	}
	else
	    return 0;
    }

    increment_written(rval);
    return rval;
}
