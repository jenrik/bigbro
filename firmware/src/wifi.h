#pragma once

#include <ESP8266mDNS.h>
#include "display.h"
#include "led.h"

class AbstractLed;
class Display;

class WiFiHandler
{
public:
    WiFiHandler(uint8_t max_tries = 5);

    bool init(AbstractLed& led,
              Display& disp);
    
private:
    MDNSResponder m_mdns;
    uint8_t m_max_tries;
};
