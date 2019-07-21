# NMEA2000WifiGateway-with-ESP32
This repository shows how to build a NMEA2000 WiFi Gateway with voltage and temperature alarms.

This project is in a very early state. KiCad schematics will be added after the current sailing season.

The code is based on the NMEA 2000 library from Timo Tappalainen (https://github.com/ttlappalainen/NMEA2000).

The ESP32 in this projext is an ESP32 NODE MCU from AzDelivery. Pin layout for other ESP32 devices might differ.

The Gateway supports the following functions:

- Providing a WiFi Access Point for other sytemes like tablets or computer (e.g. with OpenCPN)
- Forwarding navigation information from NMEA2000 to NMEA183 as TCP stream
- Forwarding serial NMEA0183 AIS information (on RX2) as UDP broadcast stream (for Navionics on tablets, but also for OpenCPN)
- Sending battery voltage and fridge temperatur as NMEA2000 sentence
- Checking votage and teperature levels against predefined values and generating alarm via an piezo alarm buzzer

The code is based on the example in the NMEA2000 library.
In addition to the original examples it also supports Trip and Rudder information. As soon as I understanf GitHub better I will send the addtional code to Timo and the NMEA2000 library.

For the ESP32 CAN bus I used the "Waveshare SN65HVD230 Can Board" as transceiver. It works well with the ESP32.
You have to define the correct GPIO ports in the header files for the NMEA2000 library (see documentation). For the AZ Delivery ESP32 NODE MCU the pins are GPIO4 for CAN RX and GPIO2 for CAN TX. The ports may differ for other ESP32 derivates.

The ADC of the ESP32 is a bit dificult to handle. You have to set the calivarion in formation in the code according to the real values of the resistors at the ADC input of the ESP 32.

The multiplexing of AIS into the TCP streams is not final. Fulle AIS message forwarding has to be implemented yet.
But for OpenCPN it is possible to define a TCP connection (192.168.4.1:2222) and adition UDP (192.168.4.255) to have forwared NMEA0183 from NMEA2000 plus AIS NMEA0183 from AIS receiver.

Further updates will follow.










