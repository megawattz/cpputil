/*
Copyright 2009 by Walt Howard
$Id: Optargs.cc 2428 2012-08-14 15:33:13Z whoward $
*/

#include <Optargs.h>
#include <Exception.h>
#include <Logger.h>

const char* Optargs::Optargs::LongOptionDivider()
{
    return "\n\t\t\t\t\t\t\t";
}

void Optargs::SetValue(const char* key, const char* value)
{
    DeepOption* option = FindByKey(key);
    if (not option)
	throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Unknown option: %s", key));

    if (not(option->Flags & DeepOption::MULTIPLE))
	option->Values.clear();

    option->Values.push_back(value);
}


void Optargs::AddValue(const char* key, const char* value)
{
    DeepOption* option = FindByKey(key);
    if (not option)
	throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Unknown option: %s", key));

    option->Values.push_back(value);
}


void Optargs::AddValue(const char short_name, const char* value)
{
    DeepOption* option = FindByShort(short_name);

    if (not option)
	throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Unknown option: -%c", short_name));

    option->Values.push_back(value);
}


Text Optargs::Settings()
{
    Text settings;
    Text freeform;

    for(OPTIONS::iterator option(Options.begin()); option != Options.end(); ++option)
    {
	if (option->Key.empty())
	{
	    freeform = Join(option->Values.begin(), option->Values.end(), ", ");
	    continue;
	}

	if (option->Values.size() == 0)
	    continue;

	settings += StringPrintf(0, "%s=%s\n", option->Key.c_str(), Join(option->Values.begin(), option->Values.end(), ", ").c_str());
    }

    if (not GetValues("").empty())
	settings += StringPrintf(0, "non-options: %s\n", freeform.c_str());

    return settings;
}


Text Optargs::Help(const char* general, const char* example)
{
    Text output;

    output += StringPrintf(0, "%s\n\n", general);

    output += StringPrintf(0, "Option\t\t\t\tDefault\t\t\tExplanation\n");
    output += StringPrintf(0, "------\t\t\t\t-------\t\t\t-----------\n");
    for(OPTIONS::iterator option(Options.begin()); option != Options.end(); ++option)
    {
	if (option->Key.empty())
	    continue;

	output += StringPrintf(0, "%c%c\t--%-15s\t%-20s\t%s\t%s\n",
			       option->Short != ' ' ? '-' : ' ',
			       option->Short,
			       option->Key.c_str(),
			       option->Default.c_str(),
			       option->Help.c_str(),
	                       option->Flags & MULTIPLE ? "(multiple allowed)" : ""
	    );
    }

    output += StringPrintf(0, "\nexample: %s\n", example);

    return output;
}


void Optargs::SetHelp(const char* key,  const char* help)
{
    DeepOption* option = FindByKey(key);
    if (not option)
	throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Unknown option: %s", key));
    option->Help = help;
}


const char* Optargs::GetValue(const char* key, unsigned which) const
{
    const DeepOption* option = FindByKey(key);
    if (!option)
	throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "option %s not recognized", key));
    if (which >= option->Values.size())
	return "";
    return option->Values[which].c_str();
}


const char* Optargs::GetValue(const char* key, unsigned which)
{
    const DeepOption* option = FindByKey(key);
    if (!option)
	throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "option %s not recognized", key));
    if (which >= option->Values.size())
	return "";
    return option->Values[which].c_str();
}


const char* Optargs::GetValue(const char* key, const char* default_value) const
{
    THROW_ON_NULL(key);
    const DeepOption* option = FindByKey(key);
    if (!option)
	throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "option %s not recognized", key));
    if (option->Values.size() == 0)
	return default_value;
    return option->Values[0].c_str();
}


const char* Optargs::GetValue(const char* key, const char* default_value)
{
    THROW_ON_NULL(key);
    const DeepOption* option = FindByKey(key);
    if (!option)
	throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "option %s not recognized", key));
    if (option->Values.size() == 0)
	return default_value;
    return option->Values[0].c_str();
}


Enhanced<std::vector<Text> > Optargs::GetValues(const char* key) const
{
    THROW_ON_NULL(key);
    const DeepOption* option = FindByKey(key);
    if (!option)
	throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "option %s not recognized", key));
    return option->Values;
}


Enhanced<std::vector<Text> > Optargs::GetValues(const char* key)
{
    THROW_ON_NULL(key);
    const DeepOption* option = FindByKey(key);
    if (!option)
	throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "option %s not recognized", key));
    return option->Values;
}


void Optargs::LoadConfigFile()
{
    Text config_name = GetValue("Config");

    try
    {
	typedef Enhanced<std::multimap<Text, Text> > CONFIGMAP;

	CONFIGMAP config(DiskFileStream(config_name.c_str(), "RDONLY").readString(99999).c_str(), "\t ", "\n");

	for(CONFIGMAP::const_iterator i(config.begin()); i != config.end(); ++i)
	    SetValue(i->first.c_str(), i->second.c_str());
    }
    catch(Exception& ex)
    {
	// if we are merely missing the config file it's ok, otherwise throw
	ex.setExplanation(ex.getExplanation() + " while reading config file: " + config_name);
	if (ex.getSystemError() != 2)
	    throw;

	std::cout << "Warning (Not a fatal error): No Config File, Using Defaults: " << config_name << std::endl;
    }
}


void Optargs::SetConfigFileName(int argc, const char* argv[])
{
    Text procinfo = DiskFileStream("/proc/self/status", "RDONLY").readToDelimiterString("\n");
    Text progname = ScanfString(procinfo.c_str(), "%*6c%s");
    Text config_name = progname + ".conf";

    Options.push_back(DeepOption("Config", 'C', config_name.c_str(), 0, "Configuration file to use."));

    if (not argc or not argv)
    {
	SetValue("Config", ConfigFileName.c_str());
	return;
    }

    SetValue("Config", config_name.c_str());

    for (int i = 1; i < argc - 1; ++i)
	if (argv[i][0] == '-' and argv[i][1] == 'C')
	{
	    if (argv[i][2] == '\0')
		SetValue("Config", argv[++i]);
	    else
		SetValue("Config", argv[i] + 2);

	    break;
	}

    ConfigFileName = GetValue("Config");
}


void Optargs::Init(const Option* arg_block, int argc, const char* argv[])
{
    if (arg_block)
	ArgBlock = arg_block;

    Options.clear(); // this can be done more than once.

    SetConfigFileName(argc, argv);

    THROW_ON_NULL(ArgBlock);

    Options.push_back(DeepOption("Verbosity", 'V', "8", 0, Logger::ExplainLevels().c_str()));
    Options.push_back(DeepOption("Core-Dump", 'D', "0", 0, "Core dump on crashes from all signals."));

    for(const Option* option = ArgBlock; option->Key != 0; ++option)
    {
	THROW_ON_NULL(option);
	THROW_ON_NULL(option->Help);
	Options.push_back(DeepOption(option));
    }

    if (not FindByKey("Verbosity"))
	Options.push_back(DeepOption("Verbosity", 'V', "8", 0, Logger::ExplainLevels().c_str()));

    if (not FindByKey("Core-Dump"))
	Options.push_back(DeepOption("Core-Dump", 'D', "0", 0, "Core dump on crashes from all signals."));

    LoadConfigFile();

    for(OPTIONS::iterator i(Options.begin()); i != Options.end(); ++i)
    {
	// load default options if specified
	DeepOption& option = *i;
        if (option.Values.empty() and not option.Default.empty())
	    option.Values.push_back(option.Default);
    }

    if (argc and argv)
	Parse(argc, argv);
}

void Optargs::ReInit()
{
    Init();
}


const Optargs::DeepOption* Optargs::FindByKey(const char* key) const
{
    for(OPTIONS::const_iterator option(Options.begin()); option != Options.end(); ++option)
    {
	if (not strcmp(option->Key.c_str(), key))
	    return &*option;
    }
    return 0;
}


Optargs::DeepOption* Optargs::FindByKey(const char* key)
{
    for(OPTIONS::iterator option(Options.begin()); option != Options.end(); ++option)
    {
	if (not strcmp(option->Key.c_str(), key))
	    return &*option;
    }
    return 0;
}


const Optargs::DeepOption* Optargs::FindByShort(const char short_option) const
{
    for(OPTIONS::const_iterator option(Options.begin()); option != Options.end(); ++option)
    {
	if (option->Short == short_option)
	    return &*option;
    }
    return 0;
}

Optargs::DeepOption* Optargs::FindByShort(const char short_option)
{
    for(OPTIONS::iterator option(Options.begin()); option != Options.end(); ++option)
    {
	if (option->Short == short_option)
	    return &*option;
    }
    return 0;
}

void Optargs::Parse(int argc, const char* argv[])
{
    char shortname('\0');

    const char* longname("");

    for( int i = 1; i < argc; ++i)
    {
	const char* current = argv[i];

	longname = 0;

	if (*current == '-')
	{
	    current++;

	    if (*current == '-') // double dash
	    {
		current++;

		if (strchr(current, '='))
		{
		    Enhanced<std::vector<Text> > parts(current, "=", 2);
		    parts.push_back(""); // just to ensure there are TWO elements and default to "1" (so that --option will mean "true")
		    SetValue(parts[0].c_str(), parts[1].c_str());
		    continue;
		}
		else
		{
		    SetValue(current, "1");
		}

		longname = current;
		shortname = '\0';
		continue;
	    }

	    shortname = *current++;

	    if (not longname)
	    {
		DeepOption* option = FindByShort(shortname);

		if (not option)
		    throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Unknown option: %c\n\n%s", shortname, Help("", "").c_str()));

		if (option->Flags & NOARG)
		{
		    option->Values.clear();
		    option->Values.push_back("1");
                    shortname = '\0';
		    continue;
		}
	    }

	    if (*current) // value right next to option?
	    {
		DeepOption* option = FindByShort(shortname);
		if (not option)
		    throw(Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Unknown option: %c\n\n%s", shortname, Help("", "").c_str()));
		SetValue(option->Key.c_str(), current);
		shortname = 0;
	    }

	    longname = 0;
	    continue;
	}

	DeepOption* option(0);

	if (shortname)
	    option = FindByShort(shortname);

	if (longname and *longname)
	    option = FindByKey(longname);

	if (not option)
	    AddValue("", current);
	else
	    SetValue(option->Key.c_str(), current);

	shortname = '\0';
	longname = "";
    }
}
