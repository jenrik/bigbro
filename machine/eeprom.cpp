#include "eeprom.h"

#include <Arduino.h>

const char* machine_id = "Royal";

const char* ssids[] = {
    "hal9k",
    "bullestock-primary"
};
const char* passwords[] = {
    "",
    "secret"
};

int Eeprom::get_nof_ssids()
{
    return sizeof(ssids)/sizeof(ssids[0]);
}

const char* Eeprom::get_ssid(int index)
{
    return ssids[index];
}

const char* Eeprom::get_password(int index)
{
    return passwords[index];
}

const char* Eeprom::get_machine_id()
{
    return machine_id;
}

const char* Eeprom::get_api_token()
{
    return "verysecret";
}
