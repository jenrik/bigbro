#define SERIAL_DBG  0

#include <RestClient.h>

#include "cardreader.h"
#include "display.h"
#include "eeprom.h"
#include "led.h"
#include "wifi.h"

const char* VERSION = "0.0.1";

// TX is not connected
#define PIN_TX       11
#define PIN_RX       14
#define PIN_SWITCH   12
#define PIN_LED      13
#define PIN_RELAY    15


Display display;

Led<PIN_LED> rgb_led;

WiFiHandler wifi_handler;

unsigned long start_tick = millis();
bool showing_version = true;

void setup()
{
    delay(1000);
    Serial.begin(115200);
    Serial.print("Cardreader v ");
    Serial.println(VERSION);

    display.set_machine_id(Eeprom::get_machine_id());
    String s = "Version ";
    s += VERSION;
    display.set_status(s.c_str());

    // Connect to WiFi network
    wifi_handler.init(rgb_led, display);
}

CardReader reader(PIN_RX, PIN_TX, PIN_SWITCH);
String last_card_id;

void loop()
{
    reader.update();
    
    const auto card_id = reader.get_card_id();
    if (card_id != last_card_id)
    {
        last_card_id = card_id;
        if (card_id.length())
        {
            display.set_status("Card present");
            RestClient client("192.168.0.", 5000);
    
        }
        else
        {
            if (!showing_version)
                display.set_status("No card");
        }
    }

    const auto now = millis();
    if (now - start_tick > 5000)
        showing_version = false;

    delay(1);
    rgb_led.update();
}
