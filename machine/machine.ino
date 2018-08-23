// Libraries:
// https://github.com/DaKaZ/esp8266-restclient
// https://github.com/plerup/espsoftwareserial

#define SERIAL_DBG  1

#include <ArduinoJson.h>
#include <EEPROM.h>

#include "acsrestclient.h"
#include "cardreader.h"
#include "display.h"
#include "eeprom.h"
#include "led.h"
#include "wifi.h"
#include "ota.h"
#include "currentsens.h"

const char* VERSION = "0.1.2";
const char* psw_md5 = "ba1f2511fc30423bdbb183fe33f3dd0f"; // OTA: Default ID and port, 123 for password.

// TX is not connected
#define PIN_TX          11
#define PIN_RX          14
#define PIN_SWITCH      12
#define PIN_LED         13
#define PIN_RELAY       15
#define PIN_CURRENT     A0
#define PIN_DEBUG       -1
#define CURRENT_THRESH  200

Display display;

Led<PIN_LED> led;

WiFiHandler wifi_handler;

//OTA ota(psw_md5); 

Current current(PIN_CURRENT, PIN_DEBUG, CURRENT_THRESH);

unsigned long start_tick = millis();
bool showing_version = true;

bool current_sensor_present = false;

void setup()
{
    delay(1000);
    Serial.begin(115200);
    Serial.print("Machine v ");
    Serial.println(VERSION);

    pinMode(PIN_RELAY, OUTPUT);

    // We need room for machine name (not more than 20 bytes) and API token (64 bytes)
    EEPROM.begin(128);

    display.set_machine_id(Eeprom::get_machine_id().c_str());
    String s = "Version ";
    s += VERSION;
    display.set_status(s);
    
    current_sensor_present = current.sensor_present();
    if(current_sensor_present)
    {
        Serial.print("Current sensor present");
        // Calibrate current offset
        display.set_status("Calibrating");
        delay(5000); // Delay to allow things to settle
        current.calibrate();
    }
         
    // Connect to WiFi network
    wifi_handler.init(led, display);
    // Set up ota uploading
    //ota.begin();
}

CardReader reader(PIN_RX, PIN_TX, PIN_SWITCH);
String last_card_id;

void decode_line(const char* line)
{
    int i = 0;
    switch (tolower(line[i]))
    {
    case 'h':
        Serial.println("Commands:");
        Serial.println("  k  Set API token");
        Serial.println("  m  Set machine ID");
        Serial.println("  t  Send test request");
        break;
        
    case 'k':
        // Set API token
        while (line[++i] == ' ')
            ;
        Eeprom::set_api_token(line+i);
        Serial.println("API token set");
        return;

    case 'm':
        // Set machine ID
        while (line[++i] == ' ')
            ;
        Eeprom::set_machine_id(line+i);
        Serial.println("Machine ID set");
        display.set_machine_id(Eeprom::get_machine_id().c_str());
        return;

    case 't':
        {
            Serial.println("Sending test request");
            String message, user_name;
            bool allowed = false;
            int user_id = 0;
            if (!query_permission("0000BB96C5", allowed, user_name, user_id, message))
            {
                Serial.print("Error: ");
                Serial.println(message);
            }
            else
            {
                Serial.print("Success: User ");
                Serial.println(user_name);
            }
        }
        break;

    default:
        Serial.print("Unknown command: ");
        Serial.println(line);
        return;
    }
}
 
const int MAX_LINE_LENGTH = 80;
char line[MAX_LINE_LENGTH+1];
int line_len = 0;

bool query_permission(const String& card_id,
                      bool& allowed,
                      String& user_name,
                      int& user_id,
                      String& message)
{
    AcsRestClient rc("permissions");
    StaticJsonBuffer<200> jsonBuffer;
    auto& root = jsonBuffer.createObject();
    root["api_token"] = Eeprom::get_api_token();
    root["card_id"] = card_id;
    const auto status = rc.post(root);
    led.update();
    Serial.print("HTTP status ");
    Serial.println(status);
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
            message = "Bad JSON";
        }
        else
        {
            allowed = json_resp["allowed"];
            user_name = (const char*) json_resp["name"];
            user_id = json_resp["id"];
        }
        return true;
    }
    else if (status == 404)
    {
        message = "Unknown card";
        allowed = false;
        return true;
    }
    String s = "Bad HTTP reply:";
    s += String(status);
    message = s;
    return false;
}


// Printer specific variables
    uint32_t        last_calibrate;
    uint32_t        end_of_print_timer;
    const uint32_t  cooldown_time = 5*60*1000; // 5
    uint16_t        last_current_reading, current_reading;

    // Keeps track of the state of the printer.
    // S0  | Printer just turned on.
    // S1  | Print in progress
    // S2  | Print finished, cooling down.
    bool    print_state = 0;

// end
void loop()
{
    yield();
    //ota.handle();
    reader.update();
    
    if(current_sensor_present)
    {
        current.handle();
        current_reading = current.read();

        // If current has changed more than 10 mA
        if(abs(current_reading-last_current_reading) >= 10) 
        {
            last_current_reading = current_reading;
            display.set_status(String(current_reading) + " mA");
        }
        // If the printer is off, recalibrate every 5min just to kill drift.
        if(millis() - last_calibrate > 300000)
        {
            last_calibrate = millis();
            if(!digitalRead(PIN_RELAY) && current.read() != 0)
            {
                display.set_status("Calibrating");
                current.calibrate();
            }
        }
    }
    
    const auto card_id = reader.get_card_id();
    // If it's a printer, if it's printing, and it's not just done with a print.
    if(current_sensor_present && current.is_printing() && print_state<2) 
    {
        print_state = 1;
        display.set_status("Print in progress", String(current_reading) + " mA");
    }
    // If we're not in state 1, or we're not a printer, check for a card with access
    else if (card_id != last_card_id)
    {
        last_card_id = card_id;

        // If there's a card present, autheticate it
        if (card_id.length())
        {
            display.set_status("Card present");
            String message, user_name;
            bool allowed = false;
            int user_id = 0;
            if (!query_permission(card_id, allowed, user_name, user_id, message))
                display.set_status(message);
            else
            {   
                if (allowed)
                {
                    digitalWrite(PIN_RELAY, 1);
                    led.set_colour(CRGB::Green);
                }
                else
                {
                    led.set_colour(CRGB::Red);
                }
            }
            yield();
            led.set_duty_cycle(50);
            led.update();
            String name_trunc = user_name;
            if (name_trunc.length() > 12)
                name_trunc = name_trunc.substring(0, 12) + String("...");
            display.set_status(name_trunc, allowed ? "OK" : "Denied");

            AcsRestClient logger("logs");
            StaticJsonBuffer<200> jsonBuffer;
            yield();
            auto& root = jsonBuffer.createObject();
            root["api_token"] = Eeprom::get_api_token();
            auto& log = root.createNestedObject("log");
            log["user_id"] = user_id;
            if (allowed)
                log["message"] = "Successful machine access";
            else
                log["message"] = "Machine access denied";
            const auto status = logger.post(root);
            yield();
            if (status != 200)
            {
                String s = "Bad HTTP log reply:";
                s += String(status);
                display.set_status(s);
            }
            else if (status == 404)
                // Unknown card
                display.set_status("Unknown card:", card_id);
            yield();
        }
        // If it's a printer and it's just finished a print
        else if(current_sensor_present && print_state == 1)
        {
            end_of_print_timer = millis();
            print_state = 2;
        }
        else if(print_state == 2 && millis()-end_of_print_timer < cooldown_time)
        {/*Don't turn off the printer during this state*/
            display.set_status("Cooling down");
        }
        else
        {
            if (!showing_version)
            display.set_status("No card");

            digitalWrite(PIN_RELAY, 0);
            led.set_colour(CRGB::Green);
            led.set_duty_cycle(1);
            led.set_period(10);
        }
    }

    const auto now = millis();
    if (now - start_tick > 5000)
        showing_version = false;

    delay(1);
    led.update();
    yield();

    if (Serial.available())
    {
        yield();
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