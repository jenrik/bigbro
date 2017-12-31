#include "acsrestclient.h"

#include <RestClient.h>

#define SERVER "192.168.0.45"
#define USE_SSL 0

AcsRestClient::AcsRestClient(const char* p)
    : path(p)
{
}

int AcsRestClient::post(const JsonObject& obj)
{
    String s;
    obj.printTo(s);
    // Work around RestClient bug
    s = String("\r\n") + s;
    RestClient client(SERVER, 80, USE_SSL);
    client.setContentType("application/json");
    String url("/api/v1/");
    url += path;
    return client.post(url.c_str(), s.c_str(), &response);
}

String AcsRestClient::get_response() const
{
    return response;
}

