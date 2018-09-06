#pragma once

#include <controller/base.h>
#include <currentsens.h>

#define PIN_TX          11
#define PIN_RX          14
#define PIN_SWITCH      12
#define PIN_CURRENT     A0
#define PIN_DEBUG       -1
#define CURRENT_THRESH  200

class ACSController: public BaseController
{
public:
    ACSController();

    void update();

    bool relay_check();

private:
    CardReader reader;
    String last_card_id;
    bool has_card = false;
    bool has_allowed_card = false;

    Current current;

    unsigned long start_tick = millis();
    bool showing_version = true;

    bool current_sensor_present = false;

    // Printer specific variables
    uint32_t        last_calibrate;
    uint32_t        end_of_print_timer;
    const uint32_t  cooldown_time = 5*60*1000; // 5
    uint16_t        last_current_reading, current_reading;

    enum PrintState { STARTED, IN_PROGRESS, COOLING };
    PrintState print_state = STARTED;
};