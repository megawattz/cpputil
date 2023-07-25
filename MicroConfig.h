/*
Copyright 2009 by Walt Howard
$Id: MicroConfig.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <vector>
#include <string>
#include <map>

/**
 One common function that most programs need is a way to extract name/value pairs from strings. For example, parsing command line arguments, parsing
 HTTP headers or Config/INI file entries.

 The MicroConfig class is a way to simplify this.

 MicroConfig holds information programs need in a mini database which can be initialized in many ways: ini file, registry, command line, tcp
 connection, string of name value pairs, http header etc.

 For example, in an init file you might have something like this:

 [Directory]
 Rights = ReadOnly
 Maxfiles = 500
 Attribute = Upper
 Attribute = Nowhitespace

 The MicroConfig object, if initialized with this ini file would store this in an std::map of std::vectors of strings, sort of a 3 dimensional array,
 like this:

 MicroConfig._config_data[Directory][Rights][0] = "ReadOnly"
 MicroConfig._config_data[Directory][Maxfile][0] = "500"
 MicroConfig._config_data[Directory][Attribute][0] = "Upper"
 MicroConfig._config_data[Directory][Attribute][1] = "Nonwhitespace"

 Loading the MicroConfig can be done in many ways, init file, xml data, windows registry, database, enironment, command line, but extracting from it
 is always done essentially the same way. This brings consistency to Configuration data and flexibility because, instead of being stuck with only
 ini files, you can load the MicroConfig object from anywhere your imagination desires. Just add a member function to load it up from a source of
 your choice.

 I recommend loading the MicroConfig object like this:

 First load Configuration items that are in a file, registry or database.  Then, load Configuration items that are stored in environment
 variables. This lets your user temporarily override normal settings by setting environment variables.  Then, load Configuration items from the
 command line.

 If the user sets the same item from different sources, it's ok, the later item can overwrite or add to the already existing information which is
 just want you want normally.

 If you need to get as complex as a tree hierachy, just make your "category" strings look like full paths, so you can have similar data items for
 different categories:

 GetValue("logger/encryption/input", "algorithm", "RSA")
 GetValue("analyzer/encryption/input", "algorithm", "RSA")

 Just remember the data is stored as Category.Name.Itemnumber or a three dimensional array. [Category][Name][ItemNumber];

 */

class SafeVector: public std::vector<std::string>
{
    typedef std::vector<std::string> BASE;
    static std::string _empty_string;

public:

    const std::string& operator[](size_t index) const
    {
	// If asked for more than the vector has, fill it in with nulls
	if (size() <= index)
	    return _empty_string;

	return at(index);
    }

};

typedef SafeVector VALUES;
typedef std::map<std::string, VALUES> NAMES;
typedef std::map<std::string, NAMES> CATEGORIES;

class MicroConfig: public CATEGORIES
{
    static VALUES _empty_values;

public:
    MicroConfig(const char* string_value_pairs = "")
    {
        loadFromNameValuePairs(string_value_pairs);
    }

    /**
     Sets, possibly overwriting, a particular value
     */
    size_t setValue(const char* category, const char* name, const unsigned which, const char* value);

    /**
     Appends a new value. Doesn't overwrite current any current value.
     */
    size_t addValue(const char* category, const char* name, const char* value);

    // Get all the items in a name
    const VALUES& getValues(const char* category, const char* name) const;

    // Get all the names in a category
    std::vector<std::string> getNames(const char* category) const;

    // Get all the categories
    std::vector<std::string> getCategories() const;

    const std::string& getValue(const char* category, const char* name, const unsigned which,  const std::string& default_value = std::string()) const; // Get one of possibly multiple values or default if not found

    const std::string& getValue(const char* category, const char* name, const std::string& default_value) const;

    const std::string& getValue(const char* name, const unsigned which = 0) const; // get one of multiple values assuming the "" category

    // read ini file style configuration information from a file, a tcp connection, a pipe, standard in, or any file descriptor ...
    bool loadFromFileDescriptor(int file_descriptor);
    bool loadFromFile(const char* file);

    /** implement later as needed */
    void loadFromEnvironment(const char* key); // read in all environment variables and their values that start with "key".
    void loadFromCommandLine(int argc, char* argv[], const char* allowed, ...); // read configuration information from the command line permitting only the allowed strings

    /** Load config object from name value pairs like name=value\nname2: value2\n-flag\n+flag\ncategory:name=value address="123 Vine Street\nHollywood, California 92342\n"
     Note: each set of key/value pairs is separated from the other sets by a newline!!! This can parse an HTTP Header into name value pairs
     */
    void loadFromHttpHeaderPairs(const char* pairs, const char* category = "");
    void loadFromNameValuePairs(const char* pairs, const char* catgory = "");

    std::string toString(const char* category = "") const;
};

/**
 To decrease dependency on any particular Configuration system, I suggest using this macro to get your configuration information:

 #define CONFIG(CATEGORY,NAME,DEFAULT) GlobalMicroConfig.GetValue(CATEGORY,NAME,DEFAULT)

 In places where user Configuration might be considered to be too heavy, you can just #define CONFIG() to simply expand to the default value so it
 essentially becomes just setting a variable.

 #ifndef GetConfig
 #define GetConfig(CATEGORY,NAME,DEFAULT) DEFAULT
 #endif

 So your code might have this: std::string name_server = CONFIG("HOSTS", "PrimaryNameServer", "dns.ucla.edu") but if you don't want the hassle of
 letting the user configure stuff, just define CONFIG to return the default value always, which would in this case become:
 name_server = "dns.ucla.edu"

 This simple class wipes out REAMS of complexity in C++ code making it a joy to make your program user configurable.
 */
