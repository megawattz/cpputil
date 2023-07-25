#include <sys/stat.h>
#include <sys/types.h>
#include <NamedPipeStream.h>

NamedPipeStream::NamedPipeStream(const char* pipe_name, const char* options)
    : DiskFileStream(pipe_name, options)
{
}


NamedPipeStream::NamedPipeStream(const Text& pipe_name, const char* options)
    : DiskFileStream(pipe_name.c_str(), options)
{
}


void NamedPipeStream::open(const char* pipe_name, const char* options)
{
    if (pipe_name)
	set_resource(pipe_name);

    if (options)
	set_options(options);

    int options_bits = parseIntoStandardOptionsBitmask(get_options().toString().c_str());

    int fifo(-1);

    if (options_bits & O_CREAT)
    {
	options_bits &= ~O_CREAT;
	fifo = mkfifo(get_resource().c_str(),  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (fifo == -1 and errno != EEXIST)
	    throw Exception(LOCATION, "Creating named pipe: %s", get_resource().c_str());
    }

    int reader, writer;

    reader = ::open(get_resource().c_str(), options_bits | O_RDONLY | O_NONBLOCK);
    if (reader == -1)
	throw Exception(LOCATION, "Opening named pipe for read: %s", get_resource().c_str());

    writer = ::open(get_resource().c_str(), options_bits | O_WRONLY | O_NONBLOCK);
    if (writer == -1)
	throw Exception(LOCATION, "Opening named pipe for writing: %s", get_resource().c_str());

    set_descriptors(reader, writer);
}
