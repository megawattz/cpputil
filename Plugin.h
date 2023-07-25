#ifndef PLUG_IN
#define PLUG_IN

#include <Text.h>
#include <string>
#include <dlfcn.h>  //linux shared library functions
#include <Misc.h>
#include <Exception.h>

//ABSTRACT_BASE is the class that all the plugin we be being.
template <typename ABSTRACT_BASE> class Plugin
{
    Text SharedLibraryFilename;
    void* SharedLibraryHandle;
    typedef ABSTRACT_BASE* (*CreateFunction)(void);
    CreateFunction CreateFunctionPointer;

   void Load()
    {
        SharedLibraryHandle = ::dlopen(Text(SharedLibraryFilename + ".so").c_str(), RTLD_LAZY|RTLD_GLOBAL);
        if (SharedLibraryHandle == NULL)
            throw Exception(LOCATION, "Unable to load shared library: %s", SharedLibraryFilename);
    }

    CreateFunction GetSymbolAddress(const char* symbol)
    {
        if (SharedLibraryHandle == NULL)
            Load();

        void* rval = ::dlsym(SharedLibraryHandle, symbol);

        if (!rval)
            throw Exception(LOCATION, "Unable to get address of %s", symbol);

        return (CreateFunction) rval;
    }

public:
    Plugin(const char* filename)
        : SharedLibraryFilename(filename), SharedLibraryHandle(NULL), CreateFunctionPointer(NULL)
    {
    }

    ABSTRACT_BASE* Create()
    {
        if (!CreateFunctionPointer)
            CreateFunctionPointer = GetSymbolAddress("Create");

        return CreateFunctionPointer();
    }

    ~Plugin()
    {
        if (SharedLibraryHandle){
            ::dlclose(SharedLibraryHandle);
        }

        CreateFunctionPointer = NULL;
        SharedLibraryHandle = NULL;
    }
};

#endif // endif PLUG_IN
