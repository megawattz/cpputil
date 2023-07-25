/*
Copyright 2009 by Walt Howard
$Id: MiniConfig.cc 2428 2012-08-14 15:33:13Z whoward $
*/


#include <MiniConfig.h>

const Text MiniConfig::_empty_value;
const MiniConfig::VALUES MiniConfig::_empty_values;
const MiniConfig::NAMES MiniConfig::_empty_names;
const MiniConfig::CATEGORIES MiniConfig::_empty_categories;

size_t MiniConfig::setValue(const char* category, const char* name,
        const unsigned which, const char* value)
{
    VALUES& values = _config_data[category][name];
    if (values.size() <= which)
    {
        values.push_back(value);
        return values.size() - 1;
    }

    values[which] = value;
    return which;
}

/**
 Appends a new value. Doesn't overwrite current any current value.
 */
size_t MiniConfig::addValue(const char* category, const char* name,
        const char* value)
{
    _config_data[category][name].push_back(value);
    return _config_data.size() - 1;
}

// Get all the items in a name
const MiniConfig::VALUES& MiniConfig::getValues(const char* category,
        const char* name) const
{
    CATEGORIES::const_iterator names = _config_data.find(category);
    if (names == _config_data.end())
        return _empty_values;

    NAMES::const_iterator values = names->second.find(name);
    if (values == names->second.end())
        return _empty_values;

    if (values->second.empty())
        return _empty_values;

    return values->second;
}

// Get all the names in a category
//GenericContainer<std::vector<Text> > GetNames(const char* category)
std::vector<Text> MiniConfig::getNames(const char* category_string) const
{
    CATEGORIES::const_iterator category = _config_data.find(category_string);
    if (category == _config_data.end())
        return _empty_values; // this needs a vector of strings, so just reuse _empty_values

    std::vector<Text> results;
    for (NAMES::const_iterator name = category->second.begin(); name
            != category->second.end(); ++name)
        results.push_back(name->first);

    return results;
}

// Get all the categories
std::vector<Text> MiniConfig::getCategories() const
{
    std::vector<Text> categories;
    for (CATEGORIES::const_iterator i(_config_data.begin()); i
            != _config_data.end(); ++i)
        categories.push_back(i->first);
    return categories;
}

const Text& MiniConfig::getValue(const char* category, const char* name,
        const unsigned which, const Text& default_value) const // Get one of possibly multiple values or default if not found
{
    CATEGORIES::const_iterator names = _config_data.find(category);
    if (names == _config_data.end())
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

const Text& MiniConfig::getValue(const char* category, const char* name,
        const Text& default_value) const
{
    return getValue(category, name, 0, default_value);
}

const Text& MiniConfig::getValue(const char* name, const unsigned which) const // get one of multiple values assuming the "" category
{
    return getValue("", name, which);
}

void MiniConfig::loadFromFileDescriptor(int file_descriptor) // read ini file style configuration information from a file, a tcp connection, a pipe, standard in, or any file descriptor ...
{
    FILE* stream = fdopen(file_descriptor, "r");
    if (not stream)
        throw(Exception(LOCATION, "Unable to open stream"));

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
    fclose(stream);
}

void MiniConfig::loadFromHttpHeaderPairs(const char* pairs,
        const char* category)
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

Text MiniConfig::toString(const char* category) const
{
    char buffer[1024] = "";
    char *p = buffer;

    CATEGORIES::const_iterator cat = _config_data.find(category);

    if (cat == _config_data.end())
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

void MiniConfig::loadFromNameValuePairs(const char* pairs, const char* category)
{
    loadFromHttpHeaderPairs(pairs, category);
}
