/*
Copyright 2009 by Walt Howard
$Id: StackTrace.cc 2428 2012-08-14 15:33:13Z whoward $
*/

#include <Misc.h>
#include <StackTrace.h>
#include <cstdio>
#include <string>
#include <cstring>
#include <cstdlib>
#include <execinfo.h>
#include <cxxabi.h>

std::string Demangle(const char* mangled_function_prototype)
{
    char extracted_symbol[1024];
    if (std::sscanf(mangled_function_prototype, "%*[^(](%[^)]", extracted_symbol) < 1)
        return mangled_function_prototype; // Could not understand input format. Don't try.
    // If the symbol has a trailing offset (example: +04a1), split extracted symbol at the + sign
    // by inserting a '\0' thereby creating two C style strings: extracted symbol and offset + 1.
    char* offset = std::strrchr(extracted_symbol, '+');
    if (offset != NULL)
        *(offset++) = '\0';
    char demangled_symbol[1024];
    size_t demangled_symbol_length(sizeof demangled_symbol);
    int status(0);

    __cxxabiv1::__cxa_demangle(extracted_symbol, demangled_symbol, &demangled_symbol_length, &status);

    if (status != 0)
        return NO_NULL_STR(mangled_function_prototype); // Error demangling. Just return original;

    std::string demangled(demangled_symbol);
    if (offset)
        (demangled += "+") += offset;

    return demangled;
}


std::string GetBacktrace()
{
    std::string stack_trace("");
    void* stack_frames[200];
    const int stack_depth = ::backtrace(stack_frames, 20);
    char* *symbols = ::backtrace_symbols(stack_frames, stack_depth);
    for (int i = 0; i < stack_depth; ++i) {
        // Only use file name if full path is present in symbol
        const char* path_char = std::strrchr(symbols[i], '/');
        if (path_char)
            stack_trace += Demangle(path_char + 1) + "\n";
        else
            stack_trace += Demangle(symbols[i]) + "\n";
    }

    ::free(symbols);

    return stack_trace;
}
