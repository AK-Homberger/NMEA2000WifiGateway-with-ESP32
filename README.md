# NMEA2000 Wifi Gateway with ESP32
This repository shows how to build a NMEA2000 WiFi Gateway with AIS multiplexing and voltage/temperature alarms.

This project is in a very early state. KiCad schematics will be added after the current sailing season.

The code is based on the NMEA 2000 library from Timo Lappalainen (https://github.com/ttlappalainen/NMEA2000).

The ESP32 in this project is an ESP32 NODE MCU from AzDelivery. Pin layout for other ESP32 devices might differ.

The Gateway supports the following functions:

- Providing a WiFi Access Point for other systems like tablets or computer (e.g. with OpenCPN).
- Forwarding navigation information from NMEA2000 to NMEA0183 as TCP stream (including log, water temp, and rudder information).
- Calculating TWS/TWD from apparent wind information and heading/COG). This allows use of OpenCPN WindHistory Dashbord instument.
- Forwarding serial NMEA0183 AIS information (on RX2) as UDP broadcast stream (for Navionics on tablets, but also for OpenCPN).
- Mutiplexing AIS information into TCP streams (to support applications that can handle only one (TCP) connection.
- Sending battery voltage and fridge temperature as NMEA2000 sentence (engine dynamic parameter PGN, my eS75 Ramarine MFD shows this as cooling temperature and alternator voltage).
- Battery voltage is measured with the ESP32 ADC.
- Fridge temperature with a Dallas DS18B20 OneWire sensor (easily extendable with more sensors).
- True parallel processing: Reading OneWire sensor as isolated task on second core of ESP32 (sensor reading is blocking for about 750 ms).
- Checking voltage and temperature levels against predefined values and generating an alarm with a piezo alarm buzzer
- Acknowledgement of alarm with a button.
- Showing fridge temperature in web browser (nice JavaScript gauge).
- JSON interface on port 90 to request NMEA2000 data via JSON request (see M5Stack Diplay repository for example on usage.

The code is based on the examples in the NMEA2000 / NMEA0183 libraries.
In addition to the original examples, it also supports Trip, Rudder and WaterTemp information. As soon as I understand GitHub better, I will send the additional code to the NMEA2000 library for integration.

Caution: The current NMEA0183 library from Timo is not fully compliant to the NMEA0183 standard. Leading zeros for Lattitude/Longitude are missing. Some applications might have problems with the missing zeros (like the current verion of the Loogbook App from Florian Kriesche). I have updated the library and informed also Timo Lappalainen to update the library in GitHub (see issues register for the NMEA0183 library for details).

For the ESP32 CAN bus, I used the "Waveshare SN65HVD230 Can Board" as transceiver. It works well with the ESP32.
You have to define the correct GPIO ports in the header files for the NMEA2000 library (see documentation). For the Gateway, I use the pins GPIO4 for CAN RX and GPIO2 for CAN TX. This is because GPIO5 is used for SC card interface. If you don't need the SD card interface you can leve the GPIOS to standard pins (GPIO04/GPIO05).

The ADC of the ESP32 is a bit difficult to handle. You have to set the calibration information in the code according to the real values of the resistors at the ADC input of the ESP 32 (e.g. 15 for 100K / 27K which gives a range from 0 to 15 Volt).

One additional hint: I replaced the 7805 voltage regulator with a step-down converter. This is reducing the power consumption. The 7805 would require addtional cooling due to power > 1 Watt (ca. 150 mA * 9 Volt).

Further updates will follow.

![Prototype](https://github.com/AK-Homberger/NMEA2000WifiGateway-with-ESP32/blob/master/Gateway%20Prototype.JPG)

![TempGauge](https://github.com/AK-Homberger/NMEA2000WifiGateway-with-ESP32/blob/master/TempGauge.png)

# Updates:
17.10.19: Version 0.8. Increase JSON buffer improved ID calculation.

14.10.19: Version 0.7. Added JSON interface on port 90. Added unique ID.

16.09.19: Added GGA sentence in N2kDataToNMEA0183.cpp

03.08.19: Version 0.4. Added rudder, log information. Calculate TWS/TWD from apparent wind, heading and COG. Added web gauge for fridge temperature (web browser, port 80).

23.07.19: Version 0.2 fixes the full AIS message forwarding issue. I'm now using the GetMessage function from the MNEA0183 library.
