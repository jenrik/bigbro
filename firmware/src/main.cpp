#include <Arduino.h>
#include <controllers/base.h>
#include <controllers/acs.h>
#include <controllers/camera.h>
#include <controllers/printer.h>

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