/*
   Copyright 2009 by Walt Howard
   $Id: Optargs.h 2428 2012-08-14 15:33:13Z whoward $
*/

#pragma once

#include <Exception.h>
#include <Text.h>
#include <Enhanced.h>
#include <StlHelpers.h>

class Optargs
{
public:
    enum { NOARG = 1, MULTIPLE = 2 };

    struct Option
    {
	enum { NOARG = 1, MULTIPLE = 2 };

	const char* Key;  // official name AND long option
	char Short;  // short name (single character)
	const char* Default; // default value
	long Flags;
	const char* Help;
    };

    struct DeepOption
    {
    public:
	Text Key;  // official name AND long option
	char Short;  // short name (single character)
	Text Default; // default value
	long Flags;
	Text Help;
	std::vector<Text> Values;

	enum { NOARG = 1, MULTIPLE = 2, REQUIRED = 4 };

	DeepOption(const char* key, char short_opt, const char* default_value, long flags, const char* help)
	    : Key(key), Short(short_opt), Default(default_value), Flags(flags), Help(help) 	{ }

	DeepOption(const Option* option)
	    : Key(NO_NULL_STR(option->Key)), Short(option->Short), Default(NO_NULL_STR(option->Default)), Flags(option->Flags), Help(option->Help) 	{ }
    };

private:
    typedef std::vector<DeepOption> OPTIONS;
    OPTIONS Options;
    const Option* ArgBlock;
    Text ConfigFileName;
    void Parse(int argc, const char* argv[]);
    void SetConfigFileName(int argc, const char* argv[]);
    void LoadConfigFile();

public:

    void SetValue(const char* key, const char* value);

    void AddValue(const char* key, const char* value);

    void AddValue(const char short_name, const char* value);

    void SetHelp(const char* key,  const char* help);

    Text Settings();

    Text Help(const char* general, const char* example);

    const char* GetValue(const char* key, const char* default_value = "") const;

    const char* GetValue(const char* key, const char* default_value = "");

    const char* operator[](const char* key) const
    {
	return GetValue(key, "");
    }

    const char* GetValue(const char* key, unsigned which) const;

    const char* GetValue(const char* key, unsigned which);

    Enhanced<std::vector<Text> > GetValues(const char* key) const;

    Enhanced<std::vector<Text> > GetValues(const char* key);

    void Init(const Option* arg_block = 0, int argc = 0, const char* argv[] = 0);

    void ReInit();

    Optargs(Option* arg_block = 0)
    {
	if (arg_block)
	    Init(arg_block, 0, 0);
    }

    DeepOption* FindByKey(const char* key);

    DeepOption* FindByShort(const char short_option);

    const DeepOption* FindByKey(const char* key) const;

    const DeepOption* FindByShort(const char short_option) const;

    static const char* LongOptionDivider();
};
