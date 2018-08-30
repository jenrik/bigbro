#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

class Display
{
public:
    Display();

    void set_machine_id(const char* id);

    void set_status(const char* line1, const char* line2 = "");

    void set_status(const String& line1, const String& line2 = "");

    void set_network_status(const char* status);

private:    
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C* m_display;
    String m_machine_id;
    String m_network_status;
    String m_line1;
    String m_line2;

    void update();
};
