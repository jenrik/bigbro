// Libraries:
// https://github.com/DaKaZ/esp8266-restclient
// https://github.com/plerup/espsoftwareserial

#define SERIAL_DBG  0

#include <ArduinoJson.h>
#include <EEPROM.h>

#include "acsrestclient.h"
#include "cardreader.h"
#include "display.h"
#include "eeprom.h"
#include "led.h"
#include "wifi.h"

const char* VERSION = "0.1.0";

// TX is not connected
#define PIN_TX       11
#define PIN_RX       14
#define PIN_SWITCH   12
#define PIN_LED      13
#define PIN_RELAY    15

Display display;

Led<PIN_LED> led;

WiFiHandler wifi_handler;

unsigned long start_tick = millis();
bool showing_version = true;

void setup()
{
    delay(1000);
    Serial.begin(115200);
    Serial.print("Machine v ");
    Serial.println(VERSION);

    // We need room for machine name (not more than 20 bytes) and API token (64 bytes)
    EEPROM.begin(128);

    display.set_machine_id(Eeprom::get_machine_id().c_str());
    String s = "Version ";
    s += VERSION;
    display.set_status(s);

    // Connect to WiFi network
    wifi_handler.init(led, display);
}

CardReader reader(PIN_RX, PIN_TX, PIN_SWITCH);
String last_card_id;

void decode_line(const char* line)
{
    int i = 0;
    switch (tolower(line[i]))
    {
    case 'k':
        // Set API token
        Eeprom::set_api_token(line+1);
        Serial.println("API token set");
        return;

    case 'm':
        // Set machine ID
        Eeprom::set_machine_id(line+1);
        Serial.println("Machine ID set");
        display.set_machine_id(Eeprom::get_machine_id().c_str());
        return;

    default:
        Serial.print("Unknown command: ");
        Serial.println(line);
        return;
    }
}

const int MAX_LINE_LENGTH = 80;
char line[MAX_LINE_LENGTH+1];
int line_len = 0;

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
            AcsRestClient rc("permissions");
            StaticJsonBuffer<200> jsonBuffer;
            auto& root = jsonBuffer.createObject();
            root["api_token"] = Eeprom::get_api_token();
            root["card_id"] = card_id;
            display.set_status("Querying...");
            const auto status = rc.post(root);
            led.update();
            if (status == 200)
            {
                auto resp = rc.get_response();
                // Remove garbage (why is it there?)
                int i = 0;
                while ((resp[i] != '{') && (i < resp.length()))
                    ++i;
                int j = i;
                while ((resp[j] != '}') && (j < resp.length()))
                    ++j;
                resp = resp.substring(i, j+1);
                StaticJsonBuffer<200> jsonBuffer;
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
                    auto user_id = json_resp["id"];
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
                    AcsRestClient logger("logs");
                    StaticJsonBuffer<200> jsonBuffer;
                    auto& root = jsonBuffer.createObject();
                    root["api_token"] = Eeprom::get_api_token();
                    auto& log = root.createNestedObject("log");
                    log["user_id"] = user_id;
                    if (allowed)
                        log["message"] = "Successful machine access";
                    else
                        log["message"] = "Machine access denied";
                    const auto status = logger.post(root);
                    if (status != 200)
                    {
                        String s = "Bad HTTP log reply:";
                        s += String(status);
                        display.set_status(s);
                    }
                }
            }
            else if (status == 404)
                // Unknown card
                display.set_status("Unknown card:", card_id);
            else
            {
                String s = "Bad HTTP reply:";
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

    if (Serial.available())
    {
        const char c = Serial.read();
        if ((c == '\r') || (c == '\n'))
        {
            line[line_len] = 0;
            line_len = 0;
            decode_line(line);
        }
        else if (line_len < MAX_LINE_LENGTH)
            line[line_len++] = c;
        else
        {
            Serial.print("Line too long: ");
            Serial.println(line);
            line_len = 0;
        }
    }


}
