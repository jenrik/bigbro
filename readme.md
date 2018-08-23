# BigBro

BigBro is the system used by Hal9K to control access to its different advanced
machines, with RFID based access control.

The system is designed to cut power to the machines if no valid key-card is present
or in the special case of a 3D printer, no special key-card is present, 
and a print isn't ongoing.

The system is designed for use on ESP8266s, specifically the WEMOS D1 mini.
to compile and uploade this, you need PlatformIO, any version of it should work.

This example code assumes you're using platformio core. The CLI tool.

```bash
git clone https://github.com/Duckle29/bigbro.git
cd bigbro
git submodule update --init --recursive
cd firmware
platformio run -t upload
```

The platformio run command can obviosly be replaced with however you prefer usign platformio,
but it is the easiest way to show it in a readme ;)