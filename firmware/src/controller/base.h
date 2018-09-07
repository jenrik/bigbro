#pragma once

#include <Arduino.h>

#define SERIAL_DBG  1

#include <EEPROM.h>
#include <ArduinoJson.h>
#include <RestClient.h>
#include <acsrestclient.h>
#include <cardreader.h>
#include <display.h>
#include <eeprom_wrapper.h>
#include <led.h>
#include <wifi.h>
#include <ota.h>

#define PIN_LED         14
#define PIN_RELAY       15
#define MAX_LINE_LENGTH 80

class BaseController
{
public:
    BaseController(const char* psw_md5 = "202cb962ac59075b964b07152d234b70");

    void update();

    virtual bool relay_check() = 0; // Define as pure virtual function

protected:
    Display display;
    Led<PIN_LED> led;
    WiFiHandler wifi_handler;
    OTA ota; // OTA: Default ID and port, 123 for password.
    // Note: password hashes can be computed using the following commands: echo -n 123 | md5sum

    bool query_permission(const String& card_id,
					  bool& allowed,
					  String& user_name,
					  int& user_id,
					  String& message);

    int log_access(const char* msg, int user_id);

    void set_relay(bool state);
    bool get_relay();

private:
    void decode_line(const char* line);

    void handleSerial();

    char line[MAX_LINE_LENGTH + 1];
    int line_len = 0;
};