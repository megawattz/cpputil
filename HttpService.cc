#include <HttpService.h>

int HttpService::Authenticate()
{
  Text& credentials = getRequestHeaders()["Authorization"];

  if (credentials.empty())
    {
      getResponseHeaders()["WWW-Authenticate"] = "basic realm=\"http\"";
      setResponseCode(401);
      throw Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Authentication failed for %s, no credentials: %s", getRequest().c_str(), getServiceStream()->PeerAddress().c_str());
    }

    Text name_password = Base64Decode(credentials.c_str() + 6);

    typedef std::map<Text, Text> STRINGMAP;

    STRINGMAP UsersPasswords;

    UsersPasswords = Enhanced<std::map<Text, Text>>(StreamFactory("http.users", "RDONLY")->readString(9999999).c_str(), "\t ", "\r\n");

    Enhanced<std::vector<Text>> name_and_password(name_password.c_str(), ":");

    if (name_and_password.size() < 2)
    {
	getResponseHeaders()["WWW-Authenticate"] = "basic realm=\"http\"";
	setResponseCode(401);
	throw Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Authentication failed for %s, no password: %s", getRequest().c_str(), getServiceStream()->PeerAddress().c_str());
    }

    STRINGMAP::const_iterator user = UsersPasswords.find(name_and_password[0]);

    if (user == UsersPasswords.end() or (name_and_password[1] != user->second))
    {
	getResponseHeaders()["WWW-Authenticate"] = "basic realm=\"http\"";
	setResponseCode(401);
	throw Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Authentication failed by: %s: %s from: %s", name_and_password[0].c_str(), getRequest().c_str(), getServiceStream()->PeerAddress().c_str());
    }

    return 0;
}

void HttpService::GetHttp()
{
    if (not ServiceStream->isReadReady(5000))
	throw Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Http data not coming in for 5 seconds, giving up");

    Text request = ServiceStream->readToDelimiterString("\r\n");
    if (request.empty())
    {
	if (not ServiceStream->isReadReady(5000))
	    throw Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Http request not coming in for 5 seconds, giving up");

	request = ServiceStream->readToDelimiterString("\r\n");
	if (request.empty())
	    throw Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Client %s too slow. No request after 5 seconds", ServiceStream->PeerAddress().c_str());

    }

    Text headers = ServiceStream->readToDelimiterString("\r\n\r\n");
    if (request.empty())
    {
	if (not ServiceStream->isReadReady(5000))
	    throw Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Http headers not coming in for 5 seconds, giving up");

	headers = ServiceStream->readToDelimiterString("\r\n\r\n");
	if (headers.empty())
	    throw Exception(Exception::NO_SYSTEM_ERROR, LOCATION, "Client %s too slow. No headers after 5 seconds", ServiceStream->PeerAddress().c_str());
    }

    char type[2048] = "";
    char document[2048] = "";
    char protocol[2048] = "";
    sscanf(request.c_str(), "%2047s %2047s %2047s", type, document, protocol);

    if (char* query = strchr(document, '?'))
    {
	*query++ = '\0';
	Queries = TEXTMAP(query, "=", "&");
    }

    Document = document;
    Request = request;
    RequestHeaders = TEXTMAP(headers.c_str(), ": ", "\r\n");
}


HttpService::HttpService(SocketStream* stream, const char* address, const char* options)
    : ServiceStream(stream), ResponseHeadersWritten(false), ResponseCode(200)
{
    if (address)
	ServiceStream->set_resource(address);

    if (options)
	ServiceStream->set_options(options);
}

void HttpService::open(SocketStream* stream, const char* address, const char* options)
{
    if (stream)
	ServiceStream = stream;

    if (address)
	ServiceStream->set_resource(address);

    if (options)
	ServiceStream->set_options(options);

    ServiceStream->open();
}

size_t HttpService::prewrite(const Text& str)
{
    int rval = ServiceStream->write(str.size(), str.c_str());
    if (improbable(rval < str.size()))
    {
	throw(Exception(LOCATION, "Error replying to Http client:%s", ServiceStream->PeerAddress().c_str()));
    }
    else
	return 0;

    return rval;
}

size_t HttpService::write(const size_t amount, const char* const source)
{
    if (not ResponseHeadersWritten)
    {
	ResponseHeadersWritten = true;
	prewrite(StringPrintf(0, "HTTP/1.1 %d\r\n", ResponseCode));
	prewrite(JoinMap(ResponseHeaders, ": ", "\r\n"));
	prewrite("\r\n\r\n");
    }

    return ServiceStream->write(amount, source);
}

size_t HttpService::writeString(const Text& str)
{
    return write(str.size(), str.c_str());
}

HttpService* HttpService::accept()
{
    SocketStream* new_service_stream = ServiceStream->accept();

    HttpService* new_service = new HttpService(new_service_stream);

    new_service->GetHttp();

    return new_service;
}
