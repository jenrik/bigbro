// Libraries:
// https://github.com/DaKaZ/esp8266-restclient

#pragma once

#define PRINTER

#include <Arduino.h>
#include <controller/base.h>
#include <controller/acs.h>

#ifdef PRINTER
    #include <controller/printer.h>
#endif


