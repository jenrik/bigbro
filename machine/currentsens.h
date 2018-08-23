#pragma once

#include <Arduino.h>


class Current
{
    public:
		// pin = adc pin, sample_period = delay between samples in uS;
        Current(uint8_t pin, int8_t debug_pin = -1, uint16_t threshold = 200); 
        uint16_t read();
        void set_range(uint16_t range);
        void set_mv_A(uint16_t mv_per);
        void handle();
        bool is_printing();
        void calibrate(); // Run this at 0A to collect the 0A offset.
        bool sensor_present(); // Checks if there's a sensor on the board.
		void sample();

    private:
        uint8_t _pin;
        uint8_t _debug_pin;

        uint32_t _last_sample		= 0;
        uint8_t _sample_period      = 10;
		bool    _raw_buffer_filled	= 0;
		int16_t _raw_samples[100]; 
    	uint8_t _raw_sample_offset	= 0;
    	uint8_t _raw_sample_size 	= 100;
    
        int16_t _error				= 0;

        int16_t  _AC_current		= 0;
        uint16_t _v_range			= 4650; // mV   | Max ADC voltage on the pin of the divider.
        uint16_t _mv_per_A       	= 200;  // mV/A | 
        uint16_t _threshold         = 200;  // Printer is considered printing above this threshold

        uint32_t _last_above_thresh;
        uint16_t _max_below_time    = 100;  // ms

        uint16_t _p2p();
        void debug_print();
};

