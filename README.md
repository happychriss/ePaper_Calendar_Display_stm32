E-ink wifi Calendar
==================

This project contains the software bits for a e-ink display David has created, and I've modified

That is the plan:

Build a "Kitchen Calendar" by using a Waveshare eInk Display with STM32 processor. 
http://www.waveshare.com/product/4.3inch-e-Paper.htm

It will be connected to a ESP8266 via UART (serial). The ESP8266 will use its Wifi to upload latest calendar entries from my Google-Calendar (e.g. twice a day). 

Once loaded it will wake up the STM32 to update the eInk display.
By this method power consumption is limited only for some seconds activity over the day - and we a always a good view on our daily activities.

Program the eInk Display - STM32 
--------

I will try to limit the processing on the ESP8266 as much as possible and to build most logic on the STM32. 

This is possible based on the great work of this two people: 
https://davidgf.net/page/41/e-ink-wifi-display
https://github.com/X-Ryl669/wifi_display 

They have extracted the original firmware and give the possibility to program the STM32 on the eInk display.

I have adjusted the cmake-file, the development is very easy by using CLION, including remote-debugging.


Code of the ESP8266
--------
https://github.com/happychriss/eInkCalendar_esp8266_sender
The ESP should have the function to 
* connect via wifi to Google-Account 
* download the calendar information
* wake up the STM32
* push the calendar information to STM32

Development is done also in CLION under Arduino using platformio.io for maintaining the dependencies. 


Code of the STM32 (this repository)
--------

* media: Some pictures used by the screen (taken from davids project)
* stm32_application: STM32F firmware that actually drives the screen (was builtin in the board!)
* stm32f10x: ST includes and libs used to build the stm32 firmware

How to build
------------

You will need to build the FW images with their corresponding toolchaings and flash them.
If you're using MacOSX or Linux, you'll be interested in:
- https://github.com/texane/stlink for the method to upload the freshly built STM32 firmware to your board (you need a ST-LINKv2 programmer, easy to find on Ebay for 5$)
- https://launchpad.net/gcc-arm-embedded for an already compiled cross compiler (used to build the STM32 firmware)
- The Expressif SDK (If you need a prebuild disk image for MacOSX, I'll upload mine)

To reuse the display driver for another board, just redefine the macros to point to your GPIOs and you are good to go!

The server should be copied to a PHP enabled server, create a config.php and fill it. Also create a screens/ dir and chmod it to be world readable/writtable.

Schematics
----------

Using UART to connect:

- Connected RX/TX on the STM32 big white and green connector to the ESP8266's pin GPIO13 and GPIO14  (I'm using the trick to swap the UART0TX output to MTDO)
- Connected GND from the ESP8266 to the programming pin on the STM32 (see David's website for a description of this programming pins, basically it's the third)
- Connected 3.3V from the ESP8266 to the programming pin on the STM32 (it's the first pin)
- Connected RST on the STM32 big white connector (RST pin 1) on the left to the ESP8266's pin GPIO12.

There is no battery in my design, you need to plug a microUSB power adapter (or a power bank) to the ESP8266 board to power the whole system. 
I'll see if in the future I'll put back a power gating transistor to power on the STM32 from the ESP8266, but currently, it works and I don't really fix what's not broken yet.

You can plug a 3.3V UART to USB on the UART1 TX line on the ESP8266 if you want to see debug messages from the ESP (115200 bauds 8N1), or on the DOUT on the STM32 (460800 bauds 8N1)
See below for changes...


GDE043A2
--------

This thing uses the Good Diplay device codenamed GDE043A2, you may find stuff googling the web. The main issue though is that there is little support and the datasheet doesn't explain a thing, check gde043a2.c to see how the driver actually works. It seems GDE060BA works all much the same (they seem to be the same device with slightly different pinouts and screen sizes, but same resolution and probably same driver).

