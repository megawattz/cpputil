#include <TcpServiceStream.h>

TcpServiceStream::TcpServiceStream(int socket, const char* address, const char* options)
    : SocketStream(address, options, socket)
{
}


TcpServiceStream::TcpServiceStream(const char* address_including_port, const char* options) :
    SocketStream(address_including_port ? address_including_port : "", options, -2)
{
}


TcpServiceStream::TcpServiceStream(const SocketAddress& address, const char* options) :
    SocketStream(address.asString().c_str(), options)
{
}


void TcpServiceStream::open(const char* address_including_port, const char* options)
{
    if (address_including_port)
    {
	set_resource(address_including_port);
    }

    if (options)
	set_options(options);

    IpSocketAddress bind_address(get_resource().c_str());

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
	throw(Exception(LOCATION, "socket(PF_INET, SOCK_STREAM, 0) failed"));

    set_descriptors(sock, sock);

    int yes(1);
    THROW_ON_ERROR(setsockopt(get_read_fd(), SOL_SOCKET, SO_REUSEADDR,
			      &yes, sizeof(yes)));

    SetOptions();

    // allow port_range which means, "if a bind fails due to port in use, increment the port number by 1 and try again and
    // keep doing that as many times as the value of the port_range= option"
    int port_range = atoi(get_options().getValue("port_range").c_str());
    if (port_range == 0)
	port_range = 1;

    short port = bind_address.getPort();

    for(int port_offset = 0; port_offset < port_range; ++port_offset)
    {
	bind_address.setPort(port + port_offset);

	int rval = ::bind(get_read_fd(), bind_address, sizeof(sockaddr));

	if (rval != -1) // success!!
	{
	    if (-1 == ::listen(get_read_fd(), 10))
		throw Exception(LOCATION, "listen fails on %s", bind_address.asString().c_str());

	    set_resource(bind_address.asString());

	    return;
	}

	if (port_range < 1)
	    throw(Exception(LOCATION, "::bind %s", bind_address.asString().c_str()));
    }
    throw(Exception(LOCATION, "Dynamic Bind: Could not bind to %s after %d attempts", bind_address.asString().c_str(), port_range));
}


TcpServiceStream* TcpServiceStream::accept()
{
    int new_socket = ::accept(get_read_fd(), NULL, NULL);

    if (new_socket == -1)
    {
	if (errno != EAGAIN)
	    throw(Exception(LOCATION, "TCP accept failed."));
	else
	    return NULL;
    }

    TcpServiceStream* new_stream = new TcpServiceStream(new_socket, get_resource().c_str());

    return new_stream;
}

Text TcpServiceStream::PeerAddress() const
{
    sockaddr address;
    socklen_t length(sizeof(address));
    int rval = ::getpeername(get_read_fd(), &address, &length);
    if (rval == -1)
	return "NOT_CONNECTED";
    return IpSocketAddress(address).asString();
}

Text TcpServiceStream::LocalAddress() const
{
    sockaddr address;
    socklen_t length(sizeof(address));
    int rval = ::getsockname(get_read_fd(), &address, &length);
    if (rval == -1)
	return "NOT_CONNECTED";
    return IpSocketAddress(address).asString();
}
