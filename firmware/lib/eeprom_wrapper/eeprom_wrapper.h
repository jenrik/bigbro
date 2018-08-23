#pragma once

#include <Arduino.h>
#include <EEPROM.h>

class Eeprom
{
public:
    static int get_nof_ssids();

    static const char* get_ssid(int index);

    static const char* get_password(int index);

    static String get_machine_id();
    static void set_machine_id(const char*);

    static String get_api_token();
    static void set_api_token(const char*);
};
