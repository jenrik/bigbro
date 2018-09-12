#include "wifi.h"

#include "eeprom_wrapper.h"
#include "display.h"
#include "led.h"

WiFiHandler::WiFiHandler()
{
}

void WiFiHandler::init(AbstractLed& led, Display& disp)
{
    led.set_colour(CRGB::Orange);
    led.set_duty_cycle(10);
    led.set_period(1);

    String dns_name = "machine-";
    dns_name += Eeprom::get_machine_id();
    WiFi.hostname(dns_name);
    WiFi.mode(WIFI_STA);

    int index = 0;
    while (1)
    {
        led.update();

        #if SERIAL_DBG
        Serial.println();
        Serial.println();
        #endif
        const auto ssid = Eeprom::get_ssid(index);
        String s = "Trying ";
        s += ssid;
        disp.set_network_status(s.c_str());
        #if SERIAL_DBG
        Serial.println(s);
        #endif

        WiFi.begin(ssid, Eeprom::get_password(index));
        
        led.update();

        bool connected = false;
        uint32_t connection_start = millis();
        while (millis() - connection_start < 5000)
        {
            delay(0);
            led.update();

            if (WiFi.status() == WL_CONNECTED)
            {
                connected = true;
                break;
            }
        }

        if (connected)
        {
            const auto token = Eeprom::get_api_token();
            disp.set_network_status(token.length() ? "Online" : "(online)");

            #if SERIAL_DBG
            Serial.println("");
            Serial.print("Connected to ");
            Serial.println(ssid);
            #endif

            led.set_colour(CRGB::Green);
            led.set_duty_cycle(1);
            led.set_period(10);

            break;
        }

        #if SERIAL_DBG
        Serial.println("");
        #endif

        index++;
        if (index >= Eeprom::get_nof_ssids())
        {
            index = 0;
        }
    }

    led.update();
    
    // Print the IP address
    Serial.println(WiFi.localIP());

    
    led.update();

    // Set up mDNS responder
    if (!m_mdns.begin(dns_name.c_str()))
    {
        Serial.println("Error setting up mDNS responder!");
    }
    else
    {
        Serial.println("mDNS responder started");
        Serial.print("My name is ");
        Serial.println(dns_name);
    }
}
