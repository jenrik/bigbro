#include "main.h"

PrinterController* controller;

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


//#include <Arduino.h>
//#include <SoftwareSerial.h>
//
//SoftwareSerial m_serial(D5, SW_SERIAL_UNUSED_PIN);
//
//void setup()
//{
//	Serial.begin(115200);
//	m_serial.begin(9600);
//	Serial.println("\nSoftware serial test started");
//}
//
//uint8_t count = 0;
//void loop() 
//{
//  while (m_serial.available() > 0) 
//  {
//	m_serial.read();
//    Serial.print('x');
//    count++;
//  }
//
//  if(count > 40)
//  {
//    count = 0;
//    Serial.println();
//  }
//}