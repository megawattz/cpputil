#include "Plugin.h"

class Stub
{
public:
    virtual const char* Verify(const char* message) const
    {
        return message;
    }
};


extern "C" {
    Stub* Create() 
    { 
        return new Stub; 
    }
}
