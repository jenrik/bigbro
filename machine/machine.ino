// Libraries:
// https://github.com/DaKaZ/esp8266-restclient
// https://github.com/plerup/espsoftwareserial

#define SERIAL_DBG  0

#include <RestClient.h>
#include <ArduinoJson.h>

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

#define SERVER "192.168.0.45"
#define USE_SSL 0

Display display;

Led<PIN_LED> led;

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
    display.set_status(s);

    // Connect to WiFi network
    wifi_handler.init(led, display);
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
            StaticJsonBuffer<200> jsonBuffer;
            auto& root = jsonBuffer.createObject();
            root["api_token"] = Eeprom::get_api_token();
            root["card_id"] = card_id;
            String s;
            root.printTo(s);
            // Work around RestClient bug
            s = String("\r\n") + s;
            RestClient client(SERVER, 80, USE_SSL);
            client.setContentType("application/json");
            String resp;
            display.set_status("Querying...");
            auto status = client.post("/api/v1/permissions", s.c_str(), &resp);
            led.update();
            if (status == 200)
            {
                // Remove garbage (why is it there?)
                int i = 0;
                while ((resp[i] != '{') && (i < resp.length()))
                    ++i;
                int j = i;
                while ((resp[j] != '}') && (j < resp.length()))
                    ++j;
                resp = resp.substring(i, j+1);
                auto& json_resp = jsonBuffer.parseObject(resp);
                if (!json_resp.success())
                {
                    Serial.println("Bad JSON:");
                    Serial.println(resp);
                    display.set_status("Bad JSON");
                }
                else
                {
                    bool allowed = json_resp["allowed"];
                    const char* name = json_resp["name"];
                    String name_trunc = name;
                    if (name_trunc.length() > 16)
                        name_trunc = name_trunc.substring(0, 12);
                    display.set_status(name_trunc,
                                       allowed ? "OK" : "Denied");
                    digitalWrite(PIN_RELAY, 1);
                    if (allowed)
                        led.set_colour(CRGB::Green);
                    else
                        led.set_colour(CRGB::Red);
                    led.set_duty_cycle(100);
                    led.update();
                    //!! log
                }
            }
            else if (status == 404)
                // Unknown card
                display.set_status("Unknown card:", card_id);
            else
            {
                String s = "Bad reply: ";
                s += String(status);
                display.set_status(s);
            }
        }
    }

    if (!card_id.length())
    {
        if (!showing_version)
            display.set_status("No card");
        digitalWrite(PIN_RELAY, 0);
        led.set_colour(CRGB::Green);
        led.set_duty_cycle(1);
        led.set_period(10);
    }

    const auto now = millis();
    if (now - start_tick > 5000)
        showing_version = false;

    delay(1);
    led.update();
}
