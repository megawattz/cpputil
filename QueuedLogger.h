/*
Copyright 2009 by Walt Howard
$Id: Logger.h 2424 2012-08-13 19:07:09Z whoward $
*/

#pragma once

#include <Text.h>
#include <DiskFileStream.h>
#include <StreamFactory.h>

class QueuedLogger: public Logger
{
    std::deque<Text> Logs;

    QueuedLogger(const char* basename = "handle:2", const char* format = "");

    void Open(struct timeval* ts = 0);

    void write(int priority, const char* where, struct timeval* when, const char* text, size_t length);

    void Write(int priority, const char* where, struct timeval* when, const char* text, size_t length);

    void Write(int priority, const char* where, struct timeval* when, const Text& text);

    void Printf(int priority, const char* where, struct timeval* when, const char* format, ...) __attribute__((format(printf, 5, 6)));

};
