#include <Text.h>
#include <Regex.h>
#include <Serialize.h>

const Text Text::EmptyString("");
const Text Text::Empty("");

Text::Text(const char* str) : std::string(str ? str : "")
{
}

Text::Text(const char* str, int length) : std::string(str ? str : "", length)
{
}

Text::Text(const std::string& root) : std::string(root)
{
}

Text::operator const char*() const
{
    return c_str();
}

Text::operator bool() const
{
    return (not empty());
}

bool Text::operator*=(const char* other) const
{
    return strcasecmp(c_str(), other) == 0;
}

bool Text::operator%=(const Regex& other) const
{
    return other.Search(c_str());
}

void Text::serialize(Channel& channel, const char* label)
{
    SerializeBase<std::string>(channel, *this);
}

Text::~Text()
{
}
