#include "eeprom_wrapper.h"

const int MACHINE_ID_OFFSET = 0;
const int MACHINE_ID_SIZE = 32;
const int API_TOKEN_OFFSET = MACHINE_ID_SIZE;
const int API_TOKEN_SIZE = 80;

const char* ssids[] =
{
    "hal9k"
};
const char* passwords[] =
{
    "",
};

int Eeprom::get_nof_ssids()
{
    return sizeof(ssids) / sizeof(ssids[0]);
}

const char* Eeprom::get_ssid(int index)
{
    return ssids[index];
}

const char* Eeprom::get_password(int index)
{
    return passwords[index];
}

String Eeprom::get_machine_id()
{
    String s;
    int i = 0;
    while (i < MACHINE_ID_SIZE)
    {
        const auto c = static_cast<char>(EEPROM.read(MACHINE_ID_OFFSET+i));
        if (!c)
        {
            break;
        }
        s.concat(c);
        i++;
    }
    return s;
}

void Eeprom::set_machine_id(const char* id)
{
    if (strlen(id) > MACHINE_ID_SIZE)
    {
        Serial.println("ERROR: Machine ID too long");
        return;
    }

    int i = 0;
    while (i < strlen(id))
    {
        EEPROM.write(MACHINE_ID_OFFSET+i, id[i]);
        i++;
    }

    EEPROM.write(MACHINE_ID_OFFSET+i, 0);
    EEPROM.commit();
}

String Eeprom::get_api_token()
{
    String s;
    int i = 0;

    while (i < API_TOKEN_SIZE)
    {
        const auto c = static_cast<char>(EEPROM.read(API_TOKEN_OFFSET+i));
        if (!c)
        {
            break;
        }
        s.concat(c);
        ++i;
    }

    return s;
}


void Eeprom::set_api_token(const char* token)
{
    if (strlen(token) > API_TOKEN_SIZE)
    {
        Serial.println("ERROR: API token too long");
        return;
    }

    int i = 0;
    while (i < strlen(token))
    {
        EEPROM.write(API_TOKEN_OFFSET+i, token[i]);
        i++;
    }
    
    EEPROM.write(API_TOKEN_OFFSET+i, 0);
    EEPROM.commit();
}
