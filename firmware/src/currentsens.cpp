#include "currentsens.h"

Current::Current(uint8_t pin, int8_t debug_pin, uint16_t threshold)
{
    _pin = pin;
    _threshold = threshold;

    pinMode(_pin, INPUT);
    
    if(debug_pin >= 0)
    {
        _debug_pin = debug_pin;
        pinMode(_debug_pin, OUTPUT);
    }
}

bool Current::sensor_present()
{
    uint16_t average_reading = 0;
    for(uint8_t i=0; i<50; i++)
    {
        average_reading += analogRead(_pin);
        delay(5); // Delay serves as a yield as well
    }
    average_reading /= 50;
    if(average_reading > 500 && average_reading < 600)
    {
        return true;
    }
    return false;
}

bool Current::is_printing()
{
    if(read() < _threshold  &&  millis() - _last_above_thresh > _max_below_time)
    {
        return false;
    }
    else if(read() > _threshold)
    {
        _last_above_thresh = millis();
    }
    
    return true;
}

void Current::handle()
{
    if(_debug_pin >= 0)
    {
        // Send a pulse to show how often analog is sampled.
        digitalWrite(_debug_pin, HIGH);
        digitalWrite(_debug_pin, LOW);
    }
    sample();
}

void Current::calibrate()
{
    uint32_t calibrate_start = millis();
    while(millis() - calibrate_start < 5000)
    {
        yield();
        handle();
    }
    _error = _p2p();
    //Serial.print("Error: ");Serial.println(_error);
}

uint16_t Current::read()
{
    return (_p2p() - _error) * _v_range/1024 * _mv_per_A/1000 ;
}

void Current::set_range(uint16_t range)
{
    _v_range = range;
}

void Current::set_mv_A(uint16_t mv_per)
{
    _mv_per_A  =  mv_per;
}

void Current::sample()
{

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

// Private functions

uint16_t Current::_p2p()
{
    uint16_t max = 0, min = -1;

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

uint32_t serial_limiter;
void Current::debug_print()
{
    if(millis() - serial_limiter > 1000)
    {
        serial_limiter = millis();
        Serial.println(_p2p());
    }
    
}