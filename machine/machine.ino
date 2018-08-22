// Libraries: 
// https://github.com/DaKaZ/esp8266-restclient
// https://github.com/plerup/espsoftwareserial
// Library manager:
// ArduinoJson
// FastLed

#define SERIAL_DBG  1

#include <ArduinoJson.h>
#include <EEPROM.h>
extern "C" {
#include <user_interface.h>
}

#include "acsrestclient.h"
#include "cardreader.h"
#include "display.h"
#include "eeprom.h"
#include "led.h"
#include "wifi.h"

const char* VERSION = "C0.0.1";

// TX is not connected
#define PIN_TX       11
#define PIN_RX       14
#define PIN_SWITCH   12
#define PIN_LED      13
#define PIN_RELAY    15

#define QUERY_INTERVAL 1000

Display display;

Led<PIN_LED> led;

WiFiHandler wifi_handler;

unsigned long start_tick = 0;
unsigned long last_info_tick = 0;
unsigned long last_check_tick = 0;
bool showing_version = true;

String get_reset_reason()
{
    static const struct
    {
        int reason;
        const char* desc;
    } reasons[] = {
        { REASON_DEFAULT_RST, "Def" },
        { REASON_WDT_RST, "WD" },
        { REASON_EXCEPTION_RST, "Ex" },
        { REASON_SOFT_WDT_RST, "SWD" },
        { REASON_SOFT_RESTART, "Res" },
        { REASON_DEEP_SLEEP_AWAKE, "DS" },
        { REASON_EXT_SYS_RST, "Sys" }
    };
    
    const auto r = ESP.getResetInfoPtr()->reason;

    for (size_t i = 0; i < sizeof(reasons)/sizeof(reasons[0]); ++i)
    {
        if (r == reasons[i].reason)
            return reasons[i].desc;
    }
    return String(r);
}

void update_info(unsigned long now)
{
    auto rem = now/1000;
    const auto h = rem/(60*60);
    rem -= h*60*60;
    const auto m = rem/60;
    rem -= m*60;
    char buf[3+3+4+2+1+2+1+2+10];
    sprintf(buf, "RC %s Up %02d:%02d:%02d",
            get_reset_reason().c_str(), h, m, rem);
    display.set_info(buf);
}

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
    start_tick = millis();

    // Connect to WiFi network
    wifi_handler.init(led, display);
}

CardReader reader(PIN_RX, PIN_TX, PIN_SWITCH);

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
            String message;
            bool state = false;
            if (!query_camera_state(state, message))
            {
                Serial.print("Error: ");
                Serial.println(message);
            }
            else
            {
                Serial.print("State: ");
                Serial.println(state);
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

bool query_camera_state(bool& state,
                         String& message) {
    AcsRestClient rc("camera");
    
    StaticJsonBuffer<200> jsonBuffer;
    auto& root = jsonBuffer.createObject();
    root["api_token"] = Eeprom::get_api_token();

    const auto status = rc.post(root);
    led.update();
    Serial.print("HTTP status: ");
    Serial.println(status);

    if (status == 200) {
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
        if (!json_resp.success()) {
            Serial.println("Bad JSON:");
            Serial.println(resp);
            message = "Bad JSON";
        } else {
            state = json_resp["state"];
        }
        return true;
    } else {
    String s = "Bad HTTP reply:";
    s += String(status);
    message = s;
    return false;
}
}

void loop()
{
    yield();
    
    const auto now = millis();
    // Stop showing version after 5 seconds
    if (now - start_tick > 5000) {
        showing_version = false;
    }
    // Draw uptime
    if (now - last_info_tick > 1000) {
        //Serial.println("info");
        update_info(now);
        last_info_tick = now;
    }
    // Query camera status
    if (now - last_check_tick > QUERY_INTERVAL) {
        #if SERIAL_DBG
        Serial.println("Quering camera state");
        #endif
        
        String message;
        bool status = true;
        if (!query_camera_state(status, message)) {
                display.set_status(message);
            // Fail-safe: activate cameras
            #if SERIAL_DBG
            Serial.println("fail-safe mode enabled");
            #endif
            digitalWrite(PIN_RELAY, 1);
            display.set_status("ON, FAIL-SAFE mode");
            led.set_colour(CRGB::Red);
            led.set_duty_cycle(50);
            led.set_period(1);
            led.update();
        } else {
            if (status) {
                // Camera ON
                digitalWrite(PIN_RELAY, 1);
                led.set_colour(CRGB::Red);
                display.set_status("ON");
            } else {
                // Camera OFF
        digitalWrite(PIN_RELAY, 0);
        led.set_colour(CRGB::Green);
                display.set_status("OFF");
            }
            led.set_duty_cycle(100);
            led.update();
    }

        last_check_tick = now;
    }
    
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
