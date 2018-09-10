#include <controllers/base.h>

const char* VERSION = "0.1.2";

BaseController::BaseController(const char* psw_md5):
  	ota(psw_md5)
{
    Serial.begin(115200);
	Serial.print("Machine v ");
	Serial.println(VERSION);

	pinMode(PIN_RELAY, OUTPUT);
	set_relay(false);

	led.update();

	// We need room for machine name (not more than 20 bytes) and API token (64 bytes)
	EEPROM.begin(128);

	display.set_machine_id(Eeprom::get_machine_id().c_str());
	String s = "Version ";
	s += VERSION;
	display.set_status(s);
		 
	// Connect to WiFi network
	wifi_handler.init(led, display);
	// Set up ota uploading
	ota.begin();

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
	int i = 0;
	switch (tolower(line[i]))
	{
	case 'h':
		Serial.println("Commands:");
		Serial.println("  k  Set API token");
		Serial.println("  m  Set machine ID");
		Serial.println("  t  Send test request");
		break;
		
	case 'k':
		// Set API token
		while (line[++i] == ' ')
			;
		Eeprom::set_api_token(line+i);
		Serial.println("API token set");
		return;

	case 'm':
		// Set machine ID
		while (line[++i] == ' ')
			;
		Eeprom::set_machine_id(line+i);
		Serial.println("Machine ID set");
		display.set_machine_id(Eeprom::get_machine_id().c_str());
		return;

	case 't':
		test_command();
		break;

	default:
		Serial.print("Unknown command: ");
		Serial.println(line);
		return;
	}
}

void BaseController::test_command()
{
	Serial.println("Sending test request");
	String message, user_name;
	bool allowed = false;
	int user_id = 0;
	if (!query_permission("0000BB96C5", allowed, user_name, user_id, message))
	{
		Serial.print("Error: ");
		Serial.println(message);
	}
	else
	{
		Serial.print("Success: User ");
		Serial.println(user_name);
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
