#include "display.h"

#define STATUS_Y    30

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
}

void Display::set_status(const char* status)
{
    m_display->clearBuffer();
    m_display->drawStr(0, 0, m_machine_id.c_str());
    m_display->drawStr(0, STATUS_Y, status);
    m_display->sendBuffer();
}
