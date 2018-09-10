#include "ota.h"

OTA::OTA(const char* pwd_md5, const char* id, uint16_t port)
{
    m_id = id;
    m_pwd_md5 = pwd_md5;
    m_port = port;
}

void OTA::begin()
{
    ArduinoOTA.setPort(m_port);
    ArduinoOTA.setHostname(m_id);
    ArduinoOTA.setPasswordHash(m_pwd_md5);

    ArduinoOTA.onStart([]() 
    {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
        else // U_SPIFFS
            type = "filesystem";
        
        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        #if SERIAL_DBG
        Serial.println("Start updating " + type);
        #endif
    });
        
    ArduinoOTA.onEnd([]() 
    {
        #if SERIAL_DBG
        Serial.println("\nEnd");
        #endif
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) 
    {
        #if SERIAL_DBG
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        #endif
    });

    ArduinoOTA.onError([](ota_error_t error) 
    {
        #if SERIAL_DBG
        Serial.printf("Error[%u]: ", error);
        switch(error) 
        {
            case OTA_AUTH_ERROR:
                Serial.println("Auth Failed");
                break;
            case OTA_BEGIN_ERROR:
                Serial.println("Begin Failed");
                break;
            case OTA_CONNECT_ERROR:
                Serial.println("Connect Failed");
                break;
            case OTA_RECEIVE_ERROR:
                Serial.println("Receive Failed");
                break;
            case OTA_END_ERROR:
                Serial.println("End Failed");
                break;
        }
        #endif
    });

    ArduinoOTA.begin();
}

void OTA::handle()
{
    ArduinoOTA.handle();
}