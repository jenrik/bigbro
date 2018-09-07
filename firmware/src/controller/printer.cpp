#include <controller/printer.h>

PrinterController::PrinterController(): 
  ACSController(),
  current(pin_current, pin_debug, current_threshhold)
{
    current_sensor_present = current.sensor_present();
	if(current_sensor_present)
	{
		Serial.println("Current sensor present");
		// Calibrate current offset
		display.set_status("Calibrating");
		delay(2000); // Delay to allow things to settle
		current.calibrate();
		display.set_status("Ready");
	}
}

bool PrinterController::relay_check()
{
    switch(print_state)
    {
        case IDLE:
           idle(); 
            break;
        case IN_PROGRESS:
            in_progress();
            break;
        case COOLING:
            cooling();
            break;
        default:
            Serial.println("wat");
            break;
    }
}

void PrinterController::idle()
{
    if(new_card())
    {
        Serial.println("new card");
        has_allowed_card = card_allowed();
    }

    if(!has_card())
    {
        display.set_status("No card", "IDLE");
        led.set_colour(CRGB::Green);
        led.set_duty_cycle(1);
        led.set_period(10);
    }
    else
    {
        display.set_status("IDLE", 2);
    }

    if(has_allowed_card && !get_relay())
    {
        set_relay(true);
    }
    else if(!has_allowed_card)
    {
        set_relay(false);
    }

    if(current.is_printing() && has_allowed_card)
    {
        print_state = IN_PROGRESS;
        display.set_status("PRINTING");
    }
}

void PrinterController::in_progress()
{
    if(!current.is_printing())
    {
        end_of_print_timer = millis();
        print_state = COOLING;
        return;
    }
    
    if(last_current_reading != current_reading)
    {
        display.set_status( String( (int16_t)(floor(current_reading + 2.5)) ) + " mA", 2);
    }
}

void PrinterController::cooling()
{
    if(millis() - end_of_print_timer > cooldown_time)
    {
        print_state = IDLE;
    }

}

void PrinterController::update()
{
    ACSController::update();
    current.handle();
    current_reading = current.read();
}