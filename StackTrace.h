/*
Copyright 2009 by Walt Howard
$Id: StackTrace.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <string>

std::string Demangle(const char* mangled_function_prototype);

std::string GetBacktrace();
