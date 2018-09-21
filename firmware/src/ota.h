#pragma once

class OTA
{
public:
    OTA(const char* pwd_md5, const char* id = "Unconfigured bigBro", uint16_t port = 8266);
    
    void begin();
    
    void handle();

private:
    const char* m_id;
    const char* m_pwd_md5;
    uint16_t m_port; 
};

