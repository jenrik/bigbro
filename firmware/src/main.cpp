#include "main.h"

ACSController* controller;

void setup()
{
	controller = new ACSController();
}
void loop()
{
	controller->update();
}
