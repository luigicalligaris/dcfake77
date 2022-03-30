# DCFake77
DCFake77 is an emulator for the DCF77 time signal, used to synchronize clocks, watches, appliances and industrial equipment. DCFake77 exploits hardware available on common development boards to generate a 77.5 kHz signal, AM-modulated to encode the DCF77 protocol. 

## Available implementations
So far two implementations are provided:
- An ESP32 implementation based on the LED control PWM hardware, initially developed by Luigi Calligaris
- An ESP8266 implementation based on the software PWM code of the SDK, initially developed by Luigi Calligaris
- A Raspberry implementation based on the General Purpose Clock generator hardware, initially developed by Renzo Davoli

## Quick start
You'll need the following hardware:
- An ESP32 or ESP8266 development board (cheap ones - around 4$ - can be found online or in electronics shops) or a Raspberry Pi.
- A 330 ohm resistor or more (2 x 220 or 470 work fine), any power rating is fine. The setup is not picky, but don't go below 330, to keep currents acceptable for the GPIO drivers.
- An electromagnetic coil to couple the signal to the receiving device. I used a coil with around 20 turns made from old twisted pair. I recommend the diameter of the coil to be at least of the order of the distance between it and the receiver.
- Using a breadboard to assemble the setup is helpful. The ESP32/8266 can fit straight into the breadboard, but take care that most ESP32 boards do not fit very well on single-tile breadboards, leaving access to just one row of pins on one side of the board. Still, this is enough for our application. The Raspberry Pi will require female-to-male 100-mil jumper wires from the 40-pin header to the breadboard.

### ESP32 implementation
This implementation drives the PWM at 77.5 kHZ using the led PWM library.

- Choose the pin you want to use for signal output on the ESP32 (in this example, GPIO16).
- Wire the resistor to it and then wire the coil in series with the resistor to the GND pin on the ESP32.

(ESP32 GPIO16)----(330 ohm)----(COIL)----(GND ESP32)

- Power the ESP32 by connecting the USB cable to your computer.
- Launch Arduino IDE and load dcfake77-esp32.ino .
- Select the ESP32 board you're using in board settings. If you didn't do so yet, you may have to install the board in the IDE board manager.
- Set the output pin you chose before (in our case 16) as the led_pwm_pin constant in the top of the source file.
- Set username and password for your WiFi access point, so the ESP32 can fetch the current GMT time via NTP.ORG .
- Set the offsets between your time zone and GMT, for daylight saving time and summer time.
- Verify the code compiles (Ctrl+R).
- If successful, compile and upload the code (Ctrl+U).
- If all goes well, the ESP32 should connect to the WiFi, get GMT time and then start transmitting the emulated DCF77 signal via the coil.
- Some receiving devices need a few minutes (like 2-5) to validate the signal and synchronize to it. Be patient.
- Some debug information is sent through the UART interface while the code is running, you can monitor it using a program like GtkTerm.

### ESP8266 implementation
Due to limitations in the ESP8266 SDK, generating a 77.5 kHz PWM signal is not natively supported and would require re-writing a number of drivers. To make things simpler for this implementation the PWM is run at 25.833 kHz, which results in a third harmonic signal at 77.5 kHz.

Follow the same wiring instructions as the ESP32, choosing a suitable GPIO output pin, such as GPIO5 (also labeled as "D1" on a Lolin D1 Mini), and setting that in the .ino file.

(ESP8266 GPIO5)----(330 ohm)----(COIL)----(GND ESP8266)

Set the WiFi username and password in the .ino file and adjust your local time offset w.r.t. to UTC by monitoring the USB UART interface of the board with a program line GtkTerm, which will print the local time to be transmitted by the emulated DCF77 signal on each second.

### Raspberry implementation
Guide to be written
