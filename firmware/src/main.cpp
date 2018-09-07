#include "main.h"

PrinterController* controller;

void setup()
{
	controller = new PrinterController;
}
void loop()
{
	controller->update();
}
