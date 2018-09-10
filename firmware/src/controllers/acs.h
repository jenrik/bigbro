#pragma once

#include <controllers/base.h>

class ACSController: public BaseController
{
public:
    ACSController();

    void update();

    bool relay_check();

protected:
    CardReader reader;

    String last_card_id;
    String card_id;

    uint8_t pin_tx = 11, pin_rx = 14, pin_switch = 12;

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