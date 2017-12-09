#include "eeprom.h"

#include <Arduino.h>

const char* machine_id = "Royal";

const char* ssids[] = {
    "hal9k",
    "bullestock-guest"
};
const char* password = "";

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
    return password;
}

const char* Eeprom::get_machine_id()
{
    return machine_id;
}
