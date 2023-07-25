/*
Copyright 2009 by Walt Howard
$Id: MicroConfig.cc 2428 2012-08-14 15:33:13Z whoward $
*/


#include <MicroConfig.h>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

VALUES MicroConfig::_empty_values;
std::string SafeVector::_empty_string;

size_t MicroConfig::setValue(const char* category, const char* name, const unsigned which, const char* value)
{
    VALUES& values = (*this)[category][name];
    if (values.size() <= which)
    {
        values.push_back(value);
        return values.size() - 1;
    }

    values.at(which) = value;
    return which;
}

/**
 Appends a new value. Doesn't overwrite current any current value.
 */
size_t MicroConfig::addValue(const char* category, const char* name, const char* value)
{
    (*this)[category][name].push_back(value);
    return this->size() - 1;
}

// Get all the items in a name
const VALUES& MicroConfig::getValues(const char* category, const char* name) const
{
    CATEGORIES::const_iterator names = find(category);
    if (names == end())
        return _empty_values;

    NAMES::const_iterator values = names->second.find(name);
    if (values == names->second.end())
        return _empty_values;

    if (values->second.empty())
        return _empty_values;

    return values->second;
}

// Get all the names in a category
//GenericContainer<std::vector<std::string> > GetNames(const char* category)
std::vector<std::string> MicroConfig::getNames(const char* category_string) const
{
    CATEGORIES::const_iterator category = find(category_string);
    if (category == end())
        return _empty_values; // this needs a vector of strings, so just reuse _empty_values

    std::vector<std::string> results;
    for (NAMES::const_iterator name = category->second.begin(); name
            != category->second.end(); ++name)
        results.push_back(name->first);

    return results;
}

// Get all the categories
std::vector<std::string> MicroConfig::getCategories() const
{
    std::vector<std::string> categories;
    for (CATEGORIES::const_iterator i(begin()); i
            != end(); ++i)
        categories.push_back(i->first);
    return categories;
}

const std::string& MicroConfig::getValue(const char* category, const char* name, const unsigned which, const std::string& default_value) const // Get one of possibly multiple values or default if not found
{
    CATEGORIES::const_iterator names = find(category);
    if (names == end())
        return default_value;

    NAMES::const_iterator values = names->second.find(name);
    if (values == names->second.end())
        return default_value;

    if (values->second.empty())
        return default_value; // special empty string we keep as a default if user sets no default himself

    if (values->second.size() <= which)
        return default_value;

    return values->second[which];
}

const std::string& MicroConfig::getValue(const char* category, const char* name,
        const std::string& default_value) const
{
    return getValue(category, name, 0, default_value);
}

const std::string& MicroConfig::getValue(const char* name, const unsigned which) const // get one of multiple values assuming the "" category
{
    return getValue("", name, which);
}

bool MicroConfig::loadFromFile(const char* filename)
{
    int fd = ::open(filename, O_RDONLY);
    if (fd == -1)
	return false;
    loadFromFileDescriptor(fd);
    ::close(fd);
    return true;
}

bool MicroConfig::loadFromFileDescriptor(int file_descriptor) // read ini file style configuration information from a file, a tcp connection, a pipe, standard in, or any file descriptor ...
{
    FILE* stream = fdopen(file_descriptor, "r");
    if (not stream)
	return false;

    char category[60];

    while (not feof(stream))
    {
        char work_buffer[2048] = "";
        char name[60] = "";
        char value[512] = "";

        fgets(work_buffer, sizeof(work_buffer), stream); // Get a line from the file

        if (char* comment_marker = strstr(work_buffer, "##")) // ## denotes rest of line is a comment. Stick a null here is a quick way to truncate
            *comment_marker = '\0';

        if (char* comment_marker = strstr(work_buffer, "//")) // // denotes rest of line is a comment. Stick a null here is a quick way to truncate
            *comment_marker = '\0';

        // If it's a category, store it and do next iteration
        if (3 == sscanf(work_buffer, " [%60[^]]] ", category)) // read all whitspace, read a [, read anything not matching ] and store it, read ]
            continue;

        // read into "key" at most 60 characters until :=\t or space is seen. Read and discard any number of :=\t or spaces, read the remainder of the line into "value"
        if (2 == sscanf(work_buffer, " %60[^:=\t ]%*[:=\t ]%s", name, value))
        {
            addValue(category, name, value);
            continue;
        }
    }

    return true;
}

void MicroConfig::loadFromHttpHeaderPairs(const char* pairs, const char* category)
{
    char* work_string = strdupa(pairs);
    char* position;
    for (char* parameter = strtok_r(work_string, "\t \r\n", &position); parameter; parameter
            = strtok_r(NULL, "\t \r\n", &position))
    {
        char name[60] = "";
        char value[512] = "1";
        sscanf(parameter, " %60[^:=\t ]%*[:=\t \"]%511[^\"]", name, value);
        if (*name == '+')
            setValue(category, name + 1, 0, "1");
        else if (*name == '-')
            setValue(category, name + 1, 0, "");
        else
            addValue(category, name, value);
    }
}

std::string MicroConfig::toString(const char* category) const
{
    char buffer[1024] = "";
    char *p = buffer;

    CATEGORIES::const_iterator cat = find(category);

    if (cat == end())
	return buffer;

    for(NAMES::const_iterator name(cat->second.begin()); name != cat->second.end(); ++name)
    {
	for(VALUES::const_iterator value(name->second.begin()); value != name->second.end(); ++value)
	{
	    p += snprintf(p, sizeof(buffer) - (buffer - p), "%s=%s ", name->first.c_str(), value->c_str());
	}
    }

    return buffer;
}

void MicroConfig::loadFromNameValuePairs(const char* pairs, const char* category)
{
    loadFromHttpHeaderPairs(pairs, category);
}
