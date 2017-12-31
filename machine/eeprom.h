#pragma once

class Eeprom
{
public:
    static int get_nof_ssids();

    static const char* get_ssid(int index);

    static const char* get_password(int index);

    static const char* get_machine_id();

    static const char* get_api_token();
};
