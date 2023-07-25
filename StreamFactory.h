/*
Copyright 2009 by Walt Howard
*/

#pragma once

#include <Stream.h>
#include <FileDescriptorStream.h>
#include <SocketStream.h>
#include <boost/shared_ptr.hpp>

template <typename TYPE> class SharedPtr: public boost::shared_ptr<TYPE>
{

public:
    SharedPtr(TYPE* item)
	: boost::shared_ptr<TYPE>(item)
    {
    }

    SharedPtr& operator=(TYPE* item)
    {
        this->reset(item);
	return *this;
    }

    operator bool()
    {
	TYPE* e = this->get();
	return e!= 0;
    }

    SharedPtr()
	: boost::shared_ptr<TYPE>(reinterpret_cast<TYPE*>(0))
    {
    }
};

const char* url_resource(const char* url);

typedef SharedPtr<Stream> StreamPtr;
typedef SharedPtr<FileDescriptorStream> FileDescriptorStreamPtr;
typedef SharedPtr<SocketStream> SocketStreamPtr;

Stream* StreamFactoryInternal(const char* url, const char* options = NULL, const char* default_protocol = "file://");
FileDescriptorStream* FileDescriptorStreamFactoryInternal(const char* url, const char* options = NULL, const char* default_protocol = "file://");
SocketStream* SocketStreamFactoryInternal(const char* url, const char* options = NULL, const char* default_protocol = "");

StreamPtr StreamFactory(const char* url, const char* options = NULL, const char* default_protocol = "file://");
StreamPtr MakeStream(const char* url, const char* options = NULL, const char* default_protocol = "file://");

FileDescriptorStreamPtr FileDescriptorStreamFactory(const char* url, const char* options = NULL, const char* default_protocol = "file://");
SocketStreamPtr SocketStreamFactory(const char* url, const char* options = NULL, const char* default_protocol = "");

Text StreamHelp();
Text FileDescriptorStreamHelp();
Text SocketStreamHelp();
Text SocketStreamServiceHelp();

bool isDiskFile(const char* urlstr);

