#pragma once

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

class AbstractLed;

class WiFiHandler
{
public:
    WiFiHandler();

    void init(AbstractLed& led);
    
private:
    MDNSResponder m_mdns;    
};
