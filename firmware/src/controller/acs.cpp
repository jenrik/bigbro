#include <controller/acs.h>

ACSController::ACSController(): 
  BaseController(),
  reader(pin_rx, pin_tx, pin_switch)
{}

bool ACSController::relay_check()
{
	if (new_card())
	{
		has_allowed_card = false;
		if(has_card())
		{
			Serial.println("new card");
			display.set_status("Card present");

			has_allowed_card = card_allowed();

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
	return has_allowed_card;
}

void ACSController::update()
{
	BaseController::update();
	led.update();
	reader.update();
	card_id = reader.get_card_id();    
}

bool ACSController::has_card()
{
	return card_id.length() > 0;
}

bool ACSController::new_card()
{
	bool new_card = (card_id != last_card_id);
	last_card_id = card_id;
	return new_card;
}

bool ACSController::card_allowed()
{
	String message, user_name;
	int user_id = 0;
	bool allowed = false;

	if(query_permission(card_id, allowed, user_name, user_id, message))
	{
		// Allowed
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

	return allowed; 
}