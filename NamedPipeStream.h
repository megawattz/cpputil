/*
Copyright 2009 by Walt Howard
$Id: NamedPipeStream.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <DiskFileStream.h>

class NamedPipeStream: public DiskFileStream
{
public:
    NamedPipeStream(const char* pipe_name = NULL, const char* options = NULL);

    NamedPipeStream(const Text& pipe_name, const char* options = NULL);

    virtual void open(const char* pipe_name = NULL, const char* options = NULL);
};
