#include "main.h"

BaseController* controller;

void setup()
{
	//controller = new ACSController();
	controller = new PrinterController();
	//controller = new CameraController();
}
void loop()
{
	controller->update();
}
