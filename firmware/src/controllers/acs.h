#pragma once

#include <controllers/base.h>

class ACSController: public BaseController
{
public:
    ACSController();

    void update();

    bool relay_check();

    void test_command();

protected:
    CardReader reader;

    String last_card_id;
    String card_id;

    uint8_t pin_tx = SW_SERIAL_UNUSED_PIN, pin_rx = D5, pin_switch = D6;

    bool has_allowed_card = false;

    bool has_card();
    bool new_card();
    String get_card();
    bool card_allowed();
    bool query_permission(const String& card_id,
					  bool& allowed,
					  String& user_name,
					  int& user_id,
					  String& message);

private:
    unsigned long start_tick = millis();
    bool showing_version = true;
};