#ifndef DIR_H
#define DIR_H

#include <stdexcept>
#include <iterator>
#include <boost/shared_ptr.hpp>
#include <dirent.h>

class Directory
{
    typedef boost::shared_ptr<struct direct> SHARED_DIR;
    SHARED_DIR Dirent;

    // for( Directory::iterator i = Directory.begin(); i != Directory.end(); ++i)

public:
    class iterator: public std::forward_iterator_tag
    {
        Directory::SHARED_DIR Dir;

    public:
        iterator(struct dirent* dir = 0)
            : Dir(dir)
        {
        }

        dirent* operator++(int)
        {
            return readdir(Dir.get());
        }

        dirent* operator++()
        {
            return readdir(Dir.get());
        }

        iterator operator=(const iterator& i)
        {
            Dir = i.Dir;
        }

        dirent& operator*()
        {
            return *Dir;
        }
    };

    Directory(const char* directory)
        : Dirent(0, closedir)
    {
        dirent* dir = opendir(directory);

        if (not dir)
            THROW(Exception(LOCATION, "Error Opening Directory: \"%s\"", directory.c_str()));

        Dirent(dir);
    }

    dirent* begin()
    {
        dirent* dir = opendir(directory);

        if (not dir)
            THROW(Exception(LOCATION, "Error Opening Directory: \"%s\"", directory.c_str()));

        Dirent.reset(dir);
    }

    dirent* end()
    {
        return nullptr;
    }

};

#endif
