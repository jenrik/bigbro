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

    // Check for a card with access
	if (card_id.length() > 0) // Has card?
	{
        Serial.println("card");
        bool allowed = false;

        if (card_id != last_card_id) // Is new card?
        {
            Serial.println("new card");
            // If there's a card present, attempt to authenticate it
            display.set_status("Card present");

            String message, user_name;
            int user_id = 0;
            if (query_permission(card_id, allowed, user_name, user_id, message))
            {
                // Allowed
                has_allowed_card = allowed;
                if (allowed)
                {
                    led.set_colour(CRGB::Green);
                }
                else
                {
                    led.set_colour(CRGB::Red);
                }
            }
            else
            {   
                // Denied
                display.set_status(message);
            }

            yield();
            
            led.set_duty_cycle(50);
            led.update();
            
            String name_trunc = user_name;
            if (name_trunc.length() > 17)
            {
                name_trunc = name_trunc.substring(0, 14) + String("...");
            }
            display.set_status(name_trunc, allowed ? "OK" : "Denied");

            int status;
            if (allowed)
            {
                status = log_access("Successful machine access", user_id);
            }
            else
            {
                status = log_access("Machine access denied", user_id);
            }
            
            yield();

            switch(status)
            {
                case 200:
                    break;
                case 404:
                    display.set_status("Unknown card:", card_id);
                    break;
                default:
                    String s = "Bad HTTP log reply:";
                    s += String(status);
                    display.set_status(s);
                    break;
            }
            yield();
        }

        last_card_id = card_id;
        return allowed;
    }
    else
    {
        display.set_status("No card");
        led.set_colour(CRGB::Green);
        led.set_duty_cycle(1);
        led.set_period(10);
        return false;
    }
}

void PrinterController::idle()
{
    if(new_card())
    {
        has_allowed_card = card_allowed();
    }

    if(has_allowed_card && !get_relay())
    {
        set_relay(true);
    }

    if(!has_allowed_card && get_relay())
    {
        set_relay(false);
    }

    if(current.is_printing() && has_allowed_card)
    {
        print_state = IN_PROGRESS;
    }
}

void PrinterController::in_progress()
{
    if(!current.is_printing())
    {
        end_of_print_timer = millis();
        print_state = COOLING;
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
}