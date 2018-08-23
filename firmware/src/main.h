// Libraries:
// https://github.com/DaKaZ/esp8266-restclient
// https://github.com/plerup/espsoftwareserial

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
#include <currentsens.h>

const char* VERSION = "0.1.2";
const char* psw_md5 = "ba1f2511fc30423bdbb183fe33f3dd0f"; // OTA: Default ID and port, 123 for password.

// TX is not connected
#define PIN_TX          11
#define PIN_RX          14
#define PIN_SWITCH      12
#define PIN_LED         13
#define PIN_RELAY       15
#define PIN_CURRENT     A0


// Prototypes
void decode_line(const char* line);

const int MAX_LINE_LENGTH = 80;
char line[MAX_LINE_LENGTH+1];
int line_len = 0;
bool query_permission(const String& card_id,
                      bool& allowed,
                      String& user_name,
                      int& user_id,
                      String& message);



// Printer specific variables
uint32_t        last_calibrate;
uint32_t        end_of_print_timer;
const uint32_t  cooldown_time = 5*60*1000; // 5
uint16_t        last_current_reading, current_reading;

enum PrintState { STARTED, IN_PROGRESS, COOLING };
PrintState print_state = STARTED;

// end
