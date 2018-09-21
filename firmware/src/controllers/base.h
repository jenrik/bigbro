#pragma once

#include <display.h>
#include <led.h>
#include <wifi.h>
#include <ota.h>

#define PIN_LED         D7
#define PIN_RELAY       D8
#define MAX_LINE_LENGTH 100

#define OTA_PSW "202cb962ac59075b964b07152d234b70"

class BaseController
{
public:
    BaseController(const char* psw_md5 = OTA_PSW, const bool relay_upstart = false);

    virtual void update();

    virtual bool relay_check() = 0; // Define as pure virtual function

    virtual void test_command() = 0;

protected:
    Display display;
    Led<PIN_LED> led;
    WiFiHandler wifi_handler;
    OTA ota; // OTA: Default ID and port, 123 for password.
    // Note: password hashes can be computed using the following commands: echo -n 123 | md5sum

    int log_access(const char* msg, int user_id);

    void set_relay(bool state);
    bool get_relay();

private:
    void decode_line(const char* line);

    void handleSerial();

    char line[MAX_LINE_LENGTH + 1];
    int line_len = 0;
};