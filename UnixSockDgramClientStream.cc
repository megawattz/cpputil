#include <UnixSockDgramClientStream.h>

UnixSockDgramClientStream::UnixSockDgramClientStream(const char* address_including_socket, const char* options)
    : UnixSockDgramServiceStream(address_including_socket, options)
{
}

UnixSockDgramClientStream::UnixSockDgramClientStream(const UnixSockAddress& address, const char* options)
    : UnixSockDgramServiceStream(address.asString().c_str(), options)
{
}

void UnixSockDgramClientStream::open(const char* address_including_socket, const char* options)
{
    if (address_including_socket)
	set_resource(UnixSockAddress(address_including_socket).asString());

    if (options)
	set_options(options);

    UnixSockAddress connect_to(get_resource().c_str());

    BindFileName = StringPrintf(0, "%s.%4.4x", get_resource().c_str(), getpid());

    UnixSockDgramServiceStream::open(BindFileName.c_str(), get_option_string().c_str());

    set_resource(connect_to.asString().c_str());

    // udp doesn't really "connect" but this sets a default destination for packets
    if (-1 == ::connect(get_read_fd(), connect_to, sizeof(sockaddr)))
	throw(Exception(LOCATION, "::bind %s", get_resource().c_str()));

    setLastPeer(connect_to);
}

void UnixSockDgramClientStream::close()
{
    unlink(BindFileName.c_str());
}
