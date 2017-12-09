#pragma once

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

class AbstractLed;
class Display;

class WiFiHandler
{
public:
    WiFiHandler();

    void init(AbstractLed& led,
              Display& disp);
    
private:
    MDNSResponder m_mdns;    
};
