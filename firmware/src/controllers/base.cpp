#include <Arduino.h>
#include <ArduinoJson.h>
#include <acsrestclient.h>
#include <eeprom_wrapper.h>
#include <controllers/base.h>

const char* VERSION = "0.1.2";

BaseController::BaseController(const char* psw_md5, const bool relay_upstart):
  	ota(psw_md5)
{
	Serial.begin(115200);

	#if SERIAL_DBG
	Serial.print("Machine v ");
	Serial.println(VERSION);
	#endif
	
	pinMode(PIN_RELAY, OUTPUT);
	set_relay(relay_upstart);

	led.update();

	// Size of EEPROM is determined based on offsets and sizes defined in the wrapper
	Eeprom::Eeprom_wrap_begin();

	display.set_machine_id(Eeprom::get_machine_id().c_str());
	String s = "Version ";
	s += VERSION;
	display.set_status(s);
		 
	// Connect to WiFi network
	if(wifi_handler.init(led, display))
	{
		// Set up ota uploading
		ota.begin();
	}
	else
	{
		Serial.println("test");
	}

	delay(1000);
}

void BaseController::set_relay(bool state) 
{
	digitalWrite(PIN_RELAY, state ? HIGH : LOW);
}

bool BaseController::get_relay()
{
	return digitalRead(PIN_RELAY);
}

void BaseController::decode_line(const char* line)
{
	String ssid, pass;
	int i = 0;

	uint8_t index;
	uint32_t timeout_start;

	switch (tolower(line[i]))
	{
	case 'h':
		Serial.println("Commands:");
		Serial.println("  k  Set API token");
		Serial.println("  m  Set machine ID");
		Serial.println("  d  Delete an SSID/PASS set");
		Serial.println("  w  Set wifi: w SSID PASS");
		Serial.println("  l  List SSIDs");
		Serial.println("  t  Send test request");
		Serial.println("  r  Restart the ESP");
		break;
		
	case 'k':
		// Set API token
		while (line[++i] == ' ') {;}

		Eeprom::set_api_token(line+i);
		Serial.println("API token set");
		return;

	case 'm':
		// Set machine ID
		while (line[++i] == ' ') {;}

		Eeprom::set_machine_id(line+i);
		Serial.println("Machine ID set");
		display.set_machine_id(Eeprom::get_machine_id().c_str());
		return;

	case 'w':

		ssid = "";
		pass = "";
		// Add wifi AP
		while (line[++i] == ' ') {;} // Remove spaces before ssid

		while(line[i] != ' ' && line[i] != '\0') // As long as it's not a space
		{
			ssid += line[i++]; // Get the SSID
		}
		while(line[++i] == ' ') {;} // Remove spaces before password

		while(line[i] != ' ' && line[i] != '\0')
		{
			pass += line[i++];
		}

		Eeprom::set_wifi_creds(ssid.c_str(), pass.c_str());
		return;	

	case 'd':
		while (line[++i] == ' ') {;}
		Serial.println("Pick SSID to remove");
		Eeprom::list_ssids();

		index = 255;
		timeout_start = millis();

        while(!Serial.available() && millis() - timeout_start < user_input_timeout)
        {
            delay(0);
        }

		if(Serial.available())
		{
			index = Serial.read()-'0';
		}
		else
		{
			Serial.println("Userinput timeout");
			break;
		}

		Eeprom::remove_wifi_creds(index);
		break;	

	case 't':
		test_command();
		break;
	
	case 'l':
		Eeprom::list_ssids();
		break;

	case 'r':
		ESP.reset();
		break;
		
	default:
		Serial.print("Unknown command: ");
		Serial.println(line);
		return;
	}
}

void BaseController::handleSerial()
{
	if (Serial.available())
    {
        yield();
        const char c = Serial.read();
        if ((c == '\r') || (c == '\n'))
        {
			if(c=='\r')
			{
				Serial.read(); // Throw away the extra newline
			}
            line[line_len] = 0;
            line_len = 0;
            decode_line(line);
        }
        else if (line_len < MAX_LINE_LENGTH)
        {
            line[line_len++] = c;
        }
        else
        {
            Serial.print("Line too long: ");
            Serial.println(line);
            line_len = 0;
        }
    }
}

void BaseController::update()
{
	yield();
	this->ota.handle();
	this->handleSerial();

	set_relay(relay_check());
}

int BaseController::log_access(const char* msg, int user_id)
{
	AcsRestClient logger("logs");
	StaticJsonBuffer<200> jsonBuffer;
	yield();
	auto& root = jsonBuffer.createObject();
	root["api_token"] = Eeprom::get_api_token();
	auto& log = root.createNestedObject("log");
	log["user_id"] = user_id;
	log["message"] = msg;
	return logger.post(root);
}
