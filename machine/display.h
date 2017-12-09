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

    void set_status(const char* status);

private:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C* m_display;
    String m_machine_id;
};
