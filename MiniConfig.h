/*
Copyright 2009 by Walt Howard
$Id: MiniConfig.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <vector>
#include <string>
#include <map>
#include <Exception.h>

/**
 One common function that most programs need is a way to extract name/value pairs from strings. For example, parsing command line arguments, parsing
 HTTP headers or Config/INI file entries.

 The MiniConfig class is a way to simplify this.

 MiniConfig holds information programs need in a mini database which can be initialized in many ways: ini file, registry, command line, tcp
 connection, string of name value pairs, http header etc.

 For example, in an init file you might have something like this:

 [Directory]
 Rights = ReadOnly
 Maxfiles = 500
 Attribute = Upper
 Attribute = Nowhitespace

 The MiniConfig object, if initialized with this ini file would store this in an std::map of std::vectors of strings, sort of a 3 dimensional array,
 like this:

 MiniConfig._config_data[Directory][Rights][0] = "ReadOnly"
 MiniConfig._config_data[Directory][Maxfile][0] = "500"
 MiniConfig._config_data[Directory][Attribute][0] = "Upper"
 MiniConfig._config_data[Directory][Attribute][1] = "Nonwhitespace"

 Loading the MiniConfig can be done in many ways, init file, xml data, windows registry, database, enironment, command line, but extracting from it
 is always done essentially the same way. This brings consistency to Configuration data and flexibility because, instead of being stuck with only
 ini files, you can load the MiniConfig object from anywhere your imagination desires. Just add a member function to load it up from a source of
 your choice.

 I recommend loading the MiniConfig object like this:

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


class MiniConfig
{
    class Compare
    {
    public:
	bool operator()(const Text& left, const Text& right) const
	{
	    return strcasecmp(left.c_str(), right.c_str()) < 0;
	}
    };

    typedef std::vector<Text> VALUES;
    typedef std::map<Text, VALUES, Compare> NAMES;
    typedef std::map<Text, NAMES, Compare> CATEGORIES;
    CATEGORIES _config_data;

public:
    // These are used to return "no result, empty, null type responses"
    static const Text _empty_value;
    static const VALUES _empty_values;
    static const NAMES _empty_names;
    static const CATEGORIES _empty_categories;

    MiniConfig(const char* string_value_pairs = "")
    {
        loadFromNameValuePairs(string_value_pairs);
    }

    /**
     Sets, possibly overwriting, a particular value
     */
    size_t setValue(const char* category, const char* name, const unsigned which,
            const char* value);

    /**
     Appends a new value. Doesn't overwrite current any current value.
     */
    size_t addValue(const char* category, const char* name, const char* value);

    // Get all the items in a name
    const MiniConfig::VALUES
            & getValues(const char* category, const char* name) const;

    // Get all the names in a category
    std::vector<Text> getNames(const char* category) const;

    // Get all the categories
    std::vector<Text> getCategories() const;

    const Text& getValue(const char* category, const char* name,
			 const unsigned which,
			 const Text& default_value = MiniConfig::_empty_value) const; // Get one of possibly multiple values or default if not found

    const Text& getValue(const char* category, const char* name,
			 const Text& default_value) const;

    const Text& getValue(const char* name, const unsigned which = 0) const; // get one of multiple values assuming the "" category

    // read ini file style configuration information from a file, a tcp connection, a pipe, standard in, or any file descriptor ...
    void loadFromFileDescriptor(int file_descriptor);

    /** implement later as needed */
    void loadFromEnvironment(const char* key); // read in all environment variables and their values that start with "key".
    void loadFromCommandLine(int argc, char* argv[], const char* allowed, ...); // read configuration information from the command line permitting only the allowed strings

    /** Load config object from name value pairs like name=value\nname2: value2\n-flag\n+flag\ncategory:name=value address="123 Vine Street\nHollywood, California 92342\n"
     Note: each set of key/value pairs is separated from the other sets by a newline!!! This can parse an HTTP Header into name value pairs
     */
    void loadFromHttpHeaderPairs(const char* pairs, const char* category = "");
    void loadFromNameValuePairs(const char* pairs, const char* catgory = "");

    Text toString(const char* category = "") const;
};

/**
 To decrease dependency on any particular Configuration system, I suggest using this macro to get your configuration information:

 #define CONFIG(CATEGORY,NAME,DEFAULT) GlobalMiniConfig.GetValue(CATEGORY,NAME,DEFAULT)

 In places where user Configuration might be considered to be too heavy, you can just #define CONFIG() to simply expand to the default value so it
 essentially becomes just setting a variable.

 #ifndef GetConfig
 #define GetConfig(CATEGORY,NAME,DEFAULT) DEFAULT
 #endif

 So your code might have this: Text name_server = CONFIG("HOSTS", "PrimaryNameServer", "dns.ucla.edu") but if you don't want the hassle of
 letting the user configure stuff, just define CONFIG to return the default value always, which would in this case become:
 name_server = "dns.ucla.edu"

 This simple class wipes out REAMS of complexity in C++ code making it a joy to make your program user configurable.
 */
