#ifndef CPPAPP_H
#define CPPAPP_H

// Premake the basic infrastructure for a C++ command line program

#include <Optargs.h>

/* Example options array
Optargs::Option options[] =
{
    { "name-of-option,'z', "default_value",      flags, "Explanation of this option."},
    { "auto-reload",  'y', "",                   0, "Which files or directories to watch, which if changed, reload the server."},
    { "aux-log",      't', "/dev/null",          0, "Auxilliary log for use mainly by Lua."},
    { "aux-log-rotation", 'w', "1h",             0, "How often udb log files are closed and made ready for upload."},
    { "bind-address", 'b', "0.0.0.0:80",         0, "The specific ip address you want the server listening on." },
    { "close",        'w', "1",     Optargs::NOARG, "HTTP Connection 0-leave open 1-send close header."},
    { "container-dir",'j', "js/",                0, "Directory where the containers are kept."},
    { "", '\0', "", 0, "Free form options" }, // options with no name just go into the no-key array
    { 0, '\0', "", 0, "" }, // MUST have record  0 in first member position to terminate list
};
*/

class CppApp
{
    Optargs Options;
    int Argc;
    const char** Argv;

public:
    CppApp(int argc, const char** argv, Optargs::Option* options, const char* usage, const char* example)
        : Options(options), Argc(argc), Argv(argv)
    {
        try
        {
            Options.Init(options, argc, argv);
        }
       catch(const std::exception& exc)
        {
            std::cerr << exc.what() << Options.Help(usage, example) << std::endl;
            exit(-1);
        }
    }

    virtual int Run() = 0;

    GETSET(Optargs, Options);
};

#endif
