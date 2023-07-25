/*
Copyright 2009 by Walt Howard
$Id: DiskFileStream.h 2424 2012-08-13 19:07:09Z whoward $
*/

#pragma once

#include <FileDescriptorStream.h>
#include <Text.h>

/**
 Use a disk based file for a stream.
 */
class DiskFileStream: public FileDescriptorStream
{
    static Text OpenPath;

public:
    GETSET_STATIC(Text, OpenPath);

    DiskFileStream(const char* filename = NULL, const char* options = "RDWR CREAT");

    virtual void open(const char* filename = NULL, const char* options = "RDWR CREAT");

    // Disk files are ALWAYS read and write ready
    virtual bool isReadReady(const unsigned = 0);

    virtual bool isWriteReady(const unsigned = 0);

    virtual bool isFinite() const
    {
	return true;
    }

    static Text FindFileInOpenPath(const char* filename, const char* open_path = DiskFileStream::OpenPath);
};
