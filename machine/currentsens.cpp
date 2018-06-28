#include "currentsens.h"

Current::Current(uint8_t pin, uint16_t sample_period, int8_t debug_pin)
{
    _pin = pin;
    _sample_period = sample_period;

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
        delay(10); // Delay serves as a yield as well
    }
    average_reading /= 50;
    if(average_reading > 500 && average_reading < 600)
    {
        return true;
    }
    return false;
}

void Current::init()
{
    sampler.attach_ms(_sample_period, _sample);
}

uint32_t serial_limiter;
void Current::debug_print()
{
    if(millis() - serial_limiter > 1000)
    {
        serial_limiter = millis();
        Serial.println(_last_avg_peak);
    }
    
}

void Current::handle()
{
    if(_debug_pin >= 0)
    {
        // Send a pulse to show how often analog is sampled.
        digitalWrite(_debug_pin, HIGH);
        digitalWrite(_debug_pin, LOW);
    }
    _peak();
    //debug_print();
}

void Current::calibrate()
{
    uint32_t calibrate_start = millis();
    while(millis() - calibrate_start < 5000)
    {
        yield();
        handle();
    }
    _error = _last_avg_peak;
    Serial.print("Error: ");Serial.println(_error);
}

int16_t Current::read()
{
    return (_last_avg_peak - _error) * (_v_range/1024) * (1000/_mv_per_A) ;
}

void Current::set_range(uint16_t range)
{
    _v_range = range;
}

void Current::set_mv_A(uint16_t mv_per)
{
    _mv_per_A  =  mv_per;
}

// Private functions

uint16_t Current::_average(int16_t * arr, uint16_t size)
{
    uint32_t average_reading = 0;
    for(uint8_t i=0; i<size; i++)
    {
        average_reading += arr[i];
    }
    return average_reading/size;
}

void Current::_sample()
{
    _raw_samples[_raw_sample_offset] = analogRead(_pin);

    _raw_sample_offset++;
    if(_raw_sample_offset >= _raw_sample_size)
    {
        _raw_buffer_filled = true;
        _raw_sample_offset = 0;
    }
}

void Current::_peak() // stores the Peak current in an array
{
    if(_raw_buffer_filled)
    {
        _raw_buffer_filled = false;
    
        int16_t _max = _find_peak(_raw_samples, _raw_sample_size, 1);
        int16_t _min = _find_peak(_raw_samples, _raw_sample_size, 0);

        _peak_sample[_peak_sample_offset] = (_max-_min)/2;
        _peak_sample_offset++;
        
        if(_peak_sample_offset >= _peak_sample_size)
        {
            _last_avg_peak = _average(_peak_sample, _peak_sample_size);
            _peak_sample_offset = 0;
        }
    }
}

int16_t Current::_find_peak(int16_t * arr, uint16_t size, bool return_max)
{
    int16_t _peak = 0;
    for(uint16_t i=0; i<size; i++)
    {
        if(return_max)
        {
            if(arr[i] > _peak)
            {
                _peak = arr[i];
            }
        }
        else
        {
            if(arr[i] < _peak)
            {
                _peak = arr[i];
            }
        } 
    }
    return _peak;
}