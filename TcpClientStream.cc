#include <TcpClientStream.h>

TcpClientStream::TcpClientStream(const char* address_including_socket, const char* options) :
    TcpServiceStream(address_including_socket, options)
{
}

TcpClientStream::TcpClientStream(const IpSocketAddress& address, const char* options) :
    TcpServiceStream(address.operator Text().c_str(), options)
{
}

void TcpClientStream::open(const char* address_including_socket, const char* options)
{
    if (address_including_socket)
	set_resource(address_including_socket);

    if (options)
    {
	set_options(options);
	set_connect_timeout(atoi(get_options().getValue("timeout").c_str()));
    }

    IpSocketAddress connect(get_resource().c_str());

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
	throw(Exception(LOCATION, "socket(PF_INET, SOCK_STREAM, 0) failed"));

    set_descriptors(sock, sock);

    SetOptions();

    int rval = ::connect(get_write_fd(), connect, sizeof(sockaddr));

    if (rval == -1 and errno != EINPROGRESS)
	throw(Exception(LOCATION, "TCP connect failed to address: %s", address_including_socket));

    if (get_connect_timeout())
    {
	if (not isWriteReady(get_connect_timeout() * 1000))
	    throw(Exception(Exception::ERRNO_IGNORE, LOCATION, "Timeout connecting to: %s", address_including_socket));

	int errval = getSocketError();
	if (errval)
	    throw(Exception(errval, Exception::NO_SYSTEM_ERROR, LOCATION, "Connect failed to: %s", address_including_socket));
    }
}
