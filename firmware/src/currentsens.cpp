#include "currentsens.h"

//#define DEBUG_PIN 10

Current::Current(uint8_t pin)
{
    _pin = pin;

    pinMode(_pin, INPUT);
    
    #ifdef DEBUG_PIN
        pinMode(DEBUG_PIN, OUTPUT);
    #endif
}

bool Current::sensor_present()
{
    uint16_t average_reading = 0;
    for(uint8_t i=0; i<50; i++)
    {
        average_reading += analogRead(_pin);
        delay(2); // Delay serves as a yield as well
    }
    average_reading /= 50;

    return average_reading > 500 && average_reading < 600;
}

bool Current::is_printing()
{
    if(this->read() < _threshold  &&  millis() - _last_above_thresh > _max_below_time)
    {
        return false;
    }
    else if(this->read() >= _threshold)
    {
        _last_above_thresh = millis();
        return true;
    }
}

void Current::handle()
{
    #ifdef DEBUG_PIN
        // Send a pulse to show how often analog is sampled.
        digitalWrite(_debug_pin, HIGH);
        digitalWrite(_debug_pin, LOW);
    #endif

    if(_raw_sample_offset >= _raw_sample_size)
    {
        _raw_sample_offset = 0;
    }

    if(millis() - _last_sample > _sample_period)
    {
        _last_sample = millis();
        _raw_samples[_raw_sample_offset] = analogRead(A0);
        _raw_sample_offset++;
    }
}

void Current::calibrate()
{
    uint32_t calibrate_start = millis();
    while(millis() - calibrate_start < 5000)
    {;
        delay(0);
        this->handle();
    }
    _error = this->_p2p();
}

uint16_t Current::read()
{
    return (this->_p2p() - _error) * _v_range/1024 * _mv_per_A/1000;
}

// Private functions

uint16_t Current::_p2p()
{
    uint16_t max = 0, min = UINT16_MAX;

    for(uint16_t i=0; i<_raw_sample_size; i++)
    {
        if(_raw_samples[i] < min)
        {
            min = _raw_samples[i];
        }
        else if(_raw_samples[i] > max)
        {
            max = _raw_samples[i];
        }
        yield();
    }
    return max - min;
}