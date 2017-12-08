#define SERIAL_DBG  0

#include <RDM6300.h>
#include <SoftwareSerial.h>
#include <FastLED.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

const char* VERSION = "0.0.1";

// TX is not connected
#define PIN_TX       11
#define PIN_RX       14
#define PIN_SWITCH   12
#define PIN_LED      13
#define PIN_RELAY    15

#define STATUS_Y    30

SoftwareSerial swSerial(PIN_RX, PIN_TX);

U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);

CRGB rgb_led;

int sequence_len = 100;
int sequence_index = 0;
int sequence_period = 10;
int delay_counter = 0;

String machine_id = "Royal"; //!!

void setup()
{
    delay(1000);
    Serial.begin(115200);
    Serial.print("Cardreader v ");
    Serial.println(VERSION);
    swSerial.begin(9600);

    display.begin();
    display.setDrawColor(1);
    display.setFont(u8g2_font_7x14B_mr);
    display.setFontRefHeightExtendedText();
    display.setFontPosTop();
    display.setFontDirection(0);
    display.drawStr(0, 0, "Version");
    display.drawStr(15, 0, VERSION);
    display.sendBuffer();
    delay(1000);

    FastLED.addLeds<WS2811, PIN_LED, GRB>(&rgb_led, 1).setCorrection(TypicalLEDStrip);
    rgb_led.r = 255;
    FastLED.show();

    display.clearBuffer();
    display.drawStr(0, 0, machine_id.c_str());
    display.sendBuffer();
}

RDM6300 decoder;
int n = 0;
bool card_present = false;
String current_id;
unsigned long last_sent_millis = 0;

void loop()
{
    const bool switch_on = digitalRead(PIN_SWITCH);
    const auto c = swSerial.read();
    if (c > 0)
    {
        if (decoder.add_byte(c))
        {
            const auto id = decoder.get_id();
#if SERIAL_DBG
            Serial.print("ID: ");
            Serial.println(id);
#endif
            if (switch_on)
            {
                current_id = id;
#if SERIAL_DBG
                Serial.println("Saved");
#endif
                display.clearBuffer();
                display.drawStr(0, 0, machine_id.c_str());
                display.drawStr(0, STATUS_Y, "Card present");
                display.sendBuffer();
            }
#if SERIAL_DBG
            else
            {
                Serial.println("Ignored - switch off");
            }
#endif
        }
    }
    const auto now = millis();
    const auto since_last_send = now - last_sent_millis;
    if (since_last_send > 100)
    {
        last_sent_millis = now;
        if (switch_on && current_id.length())
        {
            Serial.print("Card: ");
            Serial.println(current_id);
        }
        else
        {
            current_id = "";
            display.clearBuffer();
            display.drawStr(0, 0, machine_id.c_str());
            display.drawStr(0, STATUS_Y, "No card");
            display.sendBuffer();
        }
    }

    delay(1);
    if (++delay_counter < sequence_period)
        return;
    delay_counter = 0;

    if (++sequence_index >= sequence_len)
        sequence_index = 0;
    if (sequence_index < 1)
        rgb_led = CRGB::Green;
    else
        rgb_led = CRGB::Black;
    FastLED.show();
}
