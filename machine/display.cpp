#include "display.h"

#define Y_1    15
#define Y_2    30
#define Y_3    45

Display::Display()
{
    m_display = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE);
    m_display->begin();
    m_display->clearBuffer();
    m_display->setDrawColor(1);
    m_display->setFont(u8g2_font_7x14B_mr);
    m_display->setFontRefHeightExtendedText();
    m_display->setFontPosTop();
    m_display->setFontDirection(0);
}

void Display::set_machine_id(const char* id)
{
    m_machine_id = id;
    update();
}

void Display::set_network_status(const char* status)
{
    m_network_status = status;
    update();
}

void Display::set_status(const char* line1,
                         const char* line2)
{
    m_line1 = line1;
    m_line2 = line2;
    update();
}

void Display::set_status(const String& line1,
                         const String& line2)
{
    set_status(line1.c_str(), line2.c_str());
}

void Display::update()
{
    m_display->clearBuffer();
    m_display->drawStr(0, 0, m_machine_id.c_str());
    m_display->drawStr(0, Y_1, m_network_status.c_str());
    m_display->drawStr(0, Y_2, m_line1.c_str());
    m_display->drawStr(0, Y_3, m_line2.c_str());
    m_display->sendBuffer();
}
