/*
Copyright 2009 by Walt Howard
$Id: StreamFactory.cc 2428 2012-08-14 15:33:13Z whoward $
*/

#include <StreamFactory.h>
#include <TcpClientStream.h>
#include <FileDescriptorStream.h>
#include <TcpServiceStream.h>
#include <ExecStream.h>
#include <DiskFileStream.h>
#include <StringAsStream.h>
#include <UdpClientStream.h>
#include <UdpServiceStream.h>
#include <NamedPipeStream.h>
#include <NullStream.h>
#include <NullSocketStream.h>
#include <UnixSockDgramServiceStream.h>
#include <UnixSockDgramClientStream.h>
#include <Enhanced.h>
#include <StlHelpers.h>

const char* Divider = "\n\t\t\t\t\t\t\t";

const char* url_resource(const char* url)
{
    const char* marker;

    if ((marker = static_cast<const char*>(memchr(url, ':', 12))))
    {
	// skip past one or more /
	if (*(++marker) == '/')
	    if (*(++marker) == '/')
		++marker;

	return marker;
    }

    return url;
}

Text SocketStreamHelp()
{
    Enhanced<std::vector<Text>> help(4, "tcp://test.bozo.com:40000, TIMEOUT=10\t(TCP Client with common option)",
				     "udp://127.0.0.1:514\t(UDP connection)",
				     "unix_dgram://filename\t(Unix socket)",
				     "nullsocket:\t(Throw out data)"
	);

    return Text(Divider) + Join(help, Divider);
}

Text SocketStreamServiceHelp()
{
    Enhanced<std::vector<Text>> help(3, "tcp_service://test.bozo.com:40000\t(TCP Service, wait for connections)",
				     "udp_service://0.0.0.0:514\t(UDP Service, wait for datagrams)",
				     "unix_dgram_svc://filename\t(Unix datagram socket)"
	);
    return Text(Divider) + Join(help, Divider);
}

SocketStream* SocketStreamFactoryInternal(const char* url, const char* ops, const char* default_protocol)
{
    char resource[2048];
    char options[1024];

    if (strcspn(url, ":") > 16 or !strchr(url, ':'))
	snprintf(resource, sizeof(resource), "%s%s", default_protocol, url);
    else
	strncpy(resource, url, sizeof(resource));

    strncpy(options, NO_NULL_STR(ops), sizeof options);

    // allow data following a @ in the url to be options
    char* p = strchr(resource, '@');
    if (p)
    {
	*p = '\0';
	strncpy(options, p + 1, sizeof(options));
    }

    // allow data following a , in the url to be options
    p = strchr(resource, ',');
    if (p)
    {
	*p = '\0';
	strncpy(options, p + 1, sizeof(options));
    }

    if (strstr(resource, "tcp_service:") == resource)
    {
	return new TcpServiceStream(url_resource(resource), options);
    }
    else if (strstr(resource, "tcp:") == resource)
    {
        return new TcpClientStream(url_resource(resource), options);
    }
    else if (strstr(resource, "udp:") == resource)
    {
	return new UdpClientStream(url_resource(resource), options);
    }
    else if (strstr(resource, "udp_service:") == resource)
    {
	return new UdpServiceStream(url_resource(resource), options);
    }
    else if (strstr(resource, "unixdgram:") == resource)
    {
	return new UnixSockDgramClientStream(url_resource(resource), options);
    }
    else if (strstr(resource, "unixdgramsvc:") == resource)
    {
	return new UnixSockDgramServiceStream(url_resource(resource), options);
    }
    else if (strstr(resource, "nullsocket:") == resource)
    {
	return new NullSocketStream();
    }
    else
	throw Exception(LOCATION, StringPrintf(0, "Unknown resource: \"%s\"", url).c_str(), Exception::NO_SYSTEM_ERROR);
}

Text FileDescriptorStreamHelp()
{
    Enhanced<std::vector<Text>> help(4,
				     "file:///var/log/mylog,APPEND CREATE WRITE_ONLY READ_WRITE READ_ONLY EXCL NONBLOCK DELAY FASYNC DIRECT LARGEFILE\t(Disk file with available options)",
				     "exec:///usr/local/sendmail,CAPTURE_STDOUT CAPTURE_STDIN\t(Child Process Stream)",
				     "handle:2\t(Already opened file handle stream)",
				     "STDIN, STDOUT, STDERR, handle:stdout, handle:stderr, handle:stdin\t(Standard handles)"
	);
    return Text(Divider) + Join(help, Divider) + SocketStreamHelp();
}


FileDescriptorStream* FileDescriptorStreamFactoryInternal(const char* url, const char* ops, const char* default_protocol)
{
    if (strequal(url, "STDOUT"))
    {
        return new FileDescriptorStream("STDOUT", 0, -2, 1);
    }
    else if (strequal(url, "STDERR"))
    {
        return new FileDescriptorStream("STDERR", 0, -2, 2);
    }
    else if (strequal(url, "STDIN"))
    {
        return new FileDescriptorStream("STDIN", 0, 0, -2);
    }

    char resource[2048];
    char options[1024];

    if (strcspn(url, ":") > 16 or !strchr(url, ':'))
	snprintf(resource, sizeof(resource), "%s%s", default_protocol, url);
    else
	strncpy(resource, url, sizeof(resource));

    strncpy(options, NO_NULL_STR(ops), sizeof options);

    // allow data following a @ in the url to be options
    char* p = strchr(resource, '@');
    if (p)
    {
	*p = '\0';
	strncpy(options, p + 1, sizeof(options));
    }

    // allow data following a , in the url to be options
    p = strchr(resource, ',');
    if (p)
    {
	*p = '\0';
	strncpy(options, p + 1, sizeof(options));
    }

    if (strstr(resource, "exec:") == resource)
    {
        return new ExecStream(url_resource(resource), options);
    }
    else if (not strcasecmp(resource, "handle:stdout"))
    {
        return new FileDescriptorStream("standard_out", options, -2, 1);
    }
    else if (not strcasecmp(resource, "handle:stderr"))
    {
        return new FileDescriptorStream("standard_error", options, -2, 2);
    }
    else if (not strcasecmp(resource, "handle:stdin"))
    {
        return new FileDescriptorStream("standard_in", options, 0, -2);
    }
    else if (strstr(resource, "handle:") == resource)
    {
	int descriptor = atoi(url_resource(resource));
        return new FileDescriptorStream(StringPrintf(0, "file_handle:%d", descriptor).c_str(), options, descriptor, descriptor);
    }
    else if (strstr(resource, "file:") == resource)
    {
        return new DiskFileStream(url_resource(resource), options);
    }
    else if (strstr(resource, "pipe:") == resource)
    {
        return new NamedPipeStream(url_resource(resource), options);
    }

    return SocketStreamFactoryInternal(url, ops, default_protocol);
}


Text StreamHelp()
{
    Enhanced<std::vector<Text>> help(1, "null:\t(Throw away data. Useful if a stream is required but you don't want to save)");
    return Text(Divider) + Join(help, Divider) + FileDescriptorStreamHelp();
}


Stream* StreamFactoryInternal(const char* url, const char* ops, const char* default_protocol)
{
    char resource[2048];
    char options[1024];

    if (strequal(url, "STDOUT"))
    {
        return new FileDescriptorStream("STDOUT", 0, -2, 1);
    }
    else if (strequal(url, "STDERR"))
    {
        return new FileDescriptorStream("STDERR", 0, -2, 2);
    }
    else if (strequal(url, "STDIN"))
    {
        return new FileDescriptorStream("STDIN", 0, 0, -2);
    }

    if (strcspn(url, ":") > 16 or !strchr(url, ':'))
	snprintf(resource, sizeof(resource), "%s%s", default_protocol, url);
    else
	strncpy(resource, url, sizeof(resource));

    strncpy(options, NO_NULL_STR(ops), sizeof options);

    // allow data following a @ in the url to be options
    char* p = strchr(resource, '@');
    if (p)
    {
	*p = '\0';
	strncpy(options, p + 1, sizeof(options));
    }

    // allow data following a , in the url to be options
    p = strchr(resource, ',');
    if (p)
    {
	*p = '\0';
	strncpy(options, p + 1, sizeof(options));
    }

    if (strstr(resource, "string:") == resource)
    {
        return new StringAsStream(url_resource(resource), options);
    }
    else if (strstr(resource, "null:") == resource)
    {
        return new NullStream();
    }

    return FileDescriptorStreamFactoryInternal(url, ops, default_protocol);
}

StreamPtr StreamFactory(const char* url, const char* options, const char* default_protocol)
{
    return StreamFactoryInternal(url, options, default_protocol);
}

FileDescriptorStreamPtr FileDescriptorStreamFactory(const char* url, const char* options, const char* default_protocol)
{
    return FileDescriptorStreamFactoryInternal(url, options, default_protocol);
}

SocketStreamPtr SocketStreamFactory(const char* url, const char* options, const char* default_protocol)
{
    return SocketStreamFactoryInternal(url, options, default_protocol);
}

StreamPtr MakeStream(const char* url, const char* options, const char* default_protocol)
{
    return StreamFactory(url, options, default_protocol);
}

bool isDiskFile(const char* urlstr)
{
    // a disk file or no protocol specified means a disk file
    if (strstr("STDIN|STDOUT|STDERR", urlstr))
	return false;

    if (strstr(urlstr, "file:") == urlstr)
	return true;

    if (strcspn(urlstr, ":") > 12 or not strchr(urlstr, ':'))
	return true;

    return false;
}
