#pragma once

#include <Arduino.h>
#include <Ticker.h>

Ticker sampler;

class Current
{
    public:
		// pin = adc pin, sample_period = delay between samples in uS;
        Current(uint8_t pin, uint16_t sample_period = 20, int8_t debug_pin = -1); 
        int16_t read();
        void set_range(uint16_t range);
        void set_mv_A(uint16_t mv_per);
        void handle();
		void init();
        void calibrate(); // Run this at 0A to collect the 0A offset.
        bool sensor_present(); // Checks if there's a sensor on the board.

    private:
        uint8_t _pin;
        uint8_t _debug_pin;

        uint32_t last_sample		= 0;
		volatile bool    _raw_buffer_filled	= 0;
		volatile int16_t _raw_samples[40]; 
        volatile uint8_t _raw_sample_offset = 0;
        volatile uint8_t _raw_sample_size 	= 40;	// 40 samples is 2 cycles at 50Hz
        uint8_t _peak_sample_size 	= 5; 			// 5 samples of peaks over 2 cycles is 10 cycles, a.k.a. 5Hz current meassuring
        uint8_t _peak_sample_offset = 0;  
        int16_t _peak_sample[5];
        
        int16_t _last_avg_peak 		= 0;

        int16_t _error				= 0;

        int16_t  _AC_current		= 0;
        uint16_t _v_range			= 4650; // mV   | Max ADC voltage on the pin of the divider.
        uint16_t _mv_per_A       	= 200;  // mV/A | 
        uint16_t _sample_period		= 1;  	// mS        | Time between current samples

        int16_t _find_peak(int16_t * arr, uint16_t size, bool return_max = true);
        void _peak();
        uint16_t _average(int16_t * arr, uint16_t size);
        void _sample();
        void debug_print();
};

