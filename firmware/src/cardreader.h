#pragma once

#include <RDM6300.h>
#include <SoftwareSerial.h>

class CardReader
{
public:
    CardReader(int rx_pin, int tx_pin, int sw_pin);

    void update();

    String get_card_id() const;
    
private:
    SoftwareSerial m_serial;
    RDM6300 m_decoder;
    int m_switch_pin = 0;
    bool m_card_present = false;
    String m_id;
};
