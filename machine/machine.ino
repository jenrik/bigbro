#define SERIAL_DBG  0

#include <RDM6300.h>
#include <SoftwareSerial.h>

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

SoftwareSerial swSerial(PIN_RX, PIN_TX);

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
    swSerial.begin(9600);

    display.set_machine_id(Eeprom::get_machine_id());
    String s = "Version ";
    s += VERSION;
    display.set_status(s.c_str());

    // Connect to WiFi network
    wifi_handler.init(rgb_led, display);
}

RDM6300 decoder;
int n = 0;
bool card_present = false;
String current_id;
unsigned long last_sent_millis = 0;

void loop()
{
    const bool switch_on = digitalRead(PIN_SWITCH);
    const auto c = swSerial.read();
    if (c > 0)
    {
        if (decoder.add_byte(c))
        {
            const auto id = decoder.get_id();
#if SERIAL_DBG
            Serial.print("ID: ");
            Serial.println(id);
#endif
            if (switch_on)
            {
                current_id = id;
#if SERIAL_DBG
                Serial.println("Saved");
#endif
                display.set_status("Card present");
            }
#if SERIAL_DBG
            else
            {
                Serial.println("Ignored - switch off");
            }
#endif
        }
    }
    const auto now = millis();
    const auto since_last_send = now - last_sent_millis;
    if (since_last_send > 100)
    {
        last_sent_millis = now;
        if (switch_on && current_id.length())
        {
            Serial.print("Card: ");
            Serial.println(current_id);
        }
        else
        {
            current_id = "";
            if (!showing_version)
                display.set_status("No card");
        }
    }
    if (now - start_tick > 5000)
        showing_version = false;

    delay(1);
    rgb_led.update();
}
