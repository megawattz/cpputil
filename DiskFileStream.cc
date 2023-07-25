/*
Copyright 2009 by Walt Howard
$Id: DiskFileStream.cc 2424 2012-08-13 19:07:09Z whoward $
*/

#include <DiskFileStream.h>
#include <Enhanced.h>

Text DiskFileStream::OpenPath;

DiskFileStream::DiskFileStream(const char* filename, const char* options) :
    FileDescriptorStream(filename, options ? options : "READ")
{
}


Text DiskFileStream::FindFileInOpenPath(const char* filename, const char* open_path)
{
    THROW_ON_NULL(filename);
    if (!open_path or !*open_path)
	open_path = OpenPath;

    int descriptor;

    if (Exists(filename))
	return filename;

    Enhanced<std::vector<Text>> paths(open_path, ";:");
    for (Enhanced<std::vector<Text>>::const_iterator path(paths.begin()); path != paths.end(); ++path)
    {
	Text full_path = *path + "/" + filename;

	if (Exists(full_path))
	    return full_path;
    }

    throw Exception(LOCATION, "FindInPath \"%s\" finds no file.", filename);
    return "";
}

void DiskFileStream::open(const char* filename, const char* options)
{
    if (filename)
	set_resource(filename);

    if (options)
	set_options(options);

    if (get_read_fd() > 0)
	throw(Exception(Exception::NO_SYSTEM_ERROR, Exception::NO_SYSTEM_ERROR, LOCATION, "Diskfile already opened"));

    Text open_path = get_options().getValue("path"); // allow an "open file" path;
    if (open_path.empty())
	open_path = OpenPath;

    int descriptor;

    int openflags = parseIntoStandardOptionsBitmask(get_option_string().c_str());

    const char* fname = get_resource();

    // do not follow the open path for writing creating or appending
    if (openflags & (O_CREAT|O_WRONLY|O_TRUNC|O_APPEND) )
    {
        descriptor = ::open(get_resource(), openflags, 0666);

	if (-1 == descriptor)
	    throw Exception(LOCATION, "Opening \"%s\" with options %s", get_resource().c_str(), get_options().toString().c_str());
    }
    else
    {
	Text foundfile = FindFileInOpenPath(get_resource(), OpenPath);

	set_resource(foundfile);

	descriptor = ::open(foundfile, openflags, 0666);

	if (-1 == descriptor)
	    throw Exception(LOCATION, "Opening \"%s\" with options %s", foundfile.c_str(), get_options().toString().c_str());
    }

    set_descriptors(descriptor, descriptor);
}

// Disk files are ALWAYS read and write ready
bool DiskFileStream::isReadReady(const unsigned)
{
    return true;
}


bool DiskFileStream::isWriteReady(const unsigned)
{
    return true;
}
