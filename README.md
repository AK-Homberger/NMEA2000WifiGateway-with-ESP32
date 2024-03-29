# NMEA2000 Wifi Gateway with ESP32
This repository shows how to build a NMEA2000 WiFi Gateway with AIS multiplexing and voltage/temperature alarms.
The ESP32 in this project is an ESP32 NODE MCU from AzDelivery. Pin layout for other ESP32 devices might differ.

![Prototype](https://github.com/AK-Homberger/NMEA2000WifiGateway-with-ESP32/blob/master/Gateway%20Prototype.JPG)

The Gateway supports the following functions:

- Providing a WiFi Access Point for other systems like tablets or computer (e.g. with OpenCPN).
- Alternatively it can also connect as client to a WLAN network (set WLAN_CLIENT to 1 to enable).
- Forwarding navigation information from NMEA2000 to NMEA0183 as TCP stream (including log, water temp, and rudder information).
- Calculating TWS/TWD from apparent wind information and heading/COG). This allows use of OpenCPN WindHistory Dashbord instument.
- Forwarding serial NMEA0183 AIS information (on RX2) as UDP broadcast stream (for Navionics on tablets, but also for OpenCPN).
- Mutiplexing AIS information into TCP streams (to support applications that can handle only one (TCP) connection.
- Sending battery voltage and fridge temperature as NMEA2000 sentence (engine dynamic parameter PGN, my eS75 Ramarine MFD shows this as cooling temperature and alternator voltage).
- Battery voltage is measured with the ESP32 ADC.
- Fridge temperature with a Dallas DS18B20 OneWire sensor (easily extendable with more sensors).
- True parallel processing: Reading OneWire sensor as isolated task on second core of ESP32 (sensor reading is blocking for about 750 ms).
- Checking voltage and temperature levels against predefined values and generating an alarm with a piezo alarm buzzer.
- Acknowledgement of alarm with a button.
- Showing fridge temperature in web browser (nice JavaScript gauge).
- JSON interface on port 90 to request NMEA2000 data via JSON request (see M5Stack Display repository for example on usage).

The code is based on the NMEA 2000 / NMEA0183 libraries from Timo Lappalainen (https://github.com/ttlappalainen).
Download and install the libraries from GitHub link above:

- NMEA2000
- NMEA2000_esp32
- NMEA0183

The Gateway is using the following additional libraries (to be installed with the Arduiono IDE Library Manager):

- OneWire
- OneButton
- DallasTemperature
- ArduinoJson (Please use current version 6.x.x from now on)

For the ESP32 CAN bus, I used the "Waveshare SN65HVD230 Can Board" as transceiver. It works well with the ESP32.
For the Gateway, I use the pins GPIO4 for CAN RX and GPIO5 for CAN TX. The standard configuration of the NMEA2000 library would block the Serial2 connection used for AIS.

# !!!Caution!!! 
The former versions used Pin2 for TX. But I noticed problems with the NMEA2000 bus during reboot/programming of the ESP32. It looks like pin 2 is used during reboot and also during programming. A reboot confused the wind instrument (no TWS). With this new configuration evrything works without problems.

The 12 Volt is reduced to 5 Volt with a DC Step-Down_Converter (D24V10F5, https://www.pololu.com/product/2831).

The ADC of the ESP32 is a bit difficult to handle. You have to set the calibration information in the code according to the real values of the resistors at the ADC input of the ESP 32 (e.g. 15 for 100K / 27K which gives a range from 0 to 15 Volt).

# Configuration
To adjust the program to your need please change the following values in the code:

```
#define ENABLE_DEBUG_LOG 0 // Debug log, set to 1 to enable AIS forward on USB-Serial / 2 for ADC voltage to support calibration
#define UDP_Forwarding 0   // Set to 1 for forwarding AIS from serial2 to UDP brodcast
#define HighTempAlarm 12   // Alarm level for fridge temperature (higher)
#define LowVoltageAlarm 11 // Alarm level for battery voltage (lower)

#define ADC_Calibration_Value 34.3 // The real value depends on the true resistor values for the ADC input (100K / 27 K)

#define WLAN_CLIENT 0  // Set to 1 to enable client network. 0 to act as AP only

// Wifi cofiguration Client and Access Point
const char *AP_ssid = "MyESP32";  // ESP32 as AP
const char *CL_ssid = "MyWLAN";   // ESP32 as client in network

const char *AP_password = "appassword";   // AP password. Must be longer than 7 characters
const char *CL_password = "clientpassword";  // Client password

// Put IP address details here
IPAddress AP_local_ip(192, 168, 15, 1);  // Static address for AP
IPAddress AP_gateway(192, 168, 15, 1);
IPAddress AP_subnet(255, 255, 255, 0);

IPAddress CL_local_ip(192, 168, 1, 10);  // Static address for Client Network. Please adjust to your AP IP and DHCP range!
IPAddress CL_gateway(192, 168, 1, 1);
IPAddress CL_subnet(255, 255, 255, 0);
```

Schematics and PCB details are in the KiCAD folder (to allow modifications). The board can also be ordered at Aisler.net: https://aisler.net/p/DNXXRLFU

![Schematics](https://github.com/AK-Homberger/NMEA2000WifiGateway-with-ESP32/blob/master/KiCAD/ESP32WifiAisTempVolt2/ESP32WifiAisTempVolt2.png)

![PCB](https://github.com/AK-Homberger/NMEA2000WifiGateway-with-ESP32/blob/master/KiCAD/ESP32WifiAisTempVolt2/ESP32WifiAisTempVolt2-PCB.png)

# Be careful with placing the the ESP32 on the PCB. The USB connector has to be on the right side.

Fridge temperature is also shown with a nice web gauge:

![TempGauge](https://github.com/AK-Homberger/NMEA2000WifiGateway-with-ESP32/blob/master/TempGauge.png)

# Remove the 120 ohm resistor from the transceiver
For unknown reasons, many CAN bus transceivers for the ESP32 have a 120 Ohm resistor built into them. The resistor does not belong to the devices at the ends of the stub lines, but to the ends of the backbone cable.

Whether the transceiver contains a 120 ohm resistor can be determined either by looking at the [circuit diagram](https://github.com/AK-Homberger/NMEA2000-Workshop/blob/main/Docs/SN65HVD230%20CAN%20Board_SCH.pdf) or by measuring with the multimeter.

A knife tip is suitable for removing the SMD resistor. Use it to scratch or pry off the resistance. With the transceiver shown here, place the tip of the knife in front of the resistor (between the chip and the resistor) and support the knife against the chip. Then lever the tip against the resistance and use it to scratch / loosen / break the resistance. Whichever happens first.

![Transceiver](https://github.com/AK-Homberger/NMEA2000WifiGateway-with-ESP32/blob/master/CAN-Transceiver.jpg)

It then looks like the picture. Then measure to be on the safe side. Without a 120 ohm resistor, the multimeter shows approx. 75 kOhm.

# Parts:

- Board [Link](https://aisler.net/p/DNXXRLFU)
- U1 ESP32 [Link](https://www.amazon.de/AZDelivery-NodeMCU-Development-Nachfolgermodell-ESP8266/dp/B071P98VTG/ref=sxts_sxwds-bia-wc-drs3_0?__mk_de_DE=%C3%85M%C3%85%C5%BD%C3%95%C3%91&cv_ct_cx=ESP32&dchild=1&keywords=ESP32) 
- J5 SN65HVD230 [Link](https://eckstein-shop.de/WaveshareSN65HVD230CANTransceiverBoard33V2CESDProtection)
- J2 D24V10F5 [Link](https://eckstein-shop.de/Pololu5V2C1AStep-DownSpannungsreglerD24V10F5)
- B1 Buzzer [Link](https://www.reichelt.de/de/en/developer-boards-active-piezo-buzzer-module-debo-piezo-p239111.html?&nbc=1)
- R1 Resistor 3,3 KOhm [Link](https://www.reichelt.de/de/en/carbon-film-resistor-1-4-w-5-3-3-ohm-1-4w-3-3-p1396.html?&nbc=1)
- R2 Resistor 100 KOhm [Link](https://www.reichelt.de/de/en/carbon-film-resistor-1-4w-5-100-kilo-ohms-1-4w-100k-p1337.html?&nbc=1)
- R3 Resistor 27 KOhm [Link](https://www.reichelt.de/de/en/carbon-film-resistor-1-4-w-5-27-kohm-1-4w-27k-p1392.html?&nbc=1)
- R4, R6 Resistor 4,7 KOhm [Link](https://www.reichelt.de/de/en/carbon-film-resistor-1-4-w-5-4-7-kohm-1-4w-4-7k-p1425.html?&nbc=1)
- R5 Resistor 33 KOhm [Link](https://www.reichelt.de/de/en/carbon-film-resistor-1-4-w-5-33-kohm-1-4w-33k-p1412.html?&nbc=1)
- Q1, Q2 Transistor BC547 [Link](https://www.reichelt.de/de/en/small-signal-transistors-npn-to-92-45-v-rnd-bc547-p223356.html?&nbc=1)
- D1 Diode 1N4001 [Link](https://www.reichelt.com/de/en/rectifier-diode-do41-50-v-1-a-1n-4001-p1723.html?&nbc=1)
- D2 Diode 1N4148 [Link](https://www.reichelt.de/schalt-diode-100-v-150-ma-do-35-1n-4148-p1730.html?search=1n4148)
- SW1 Push button [Link](https://www.reichelt.de/miniatur-drucktaster-0-5a-24vac-1x-ein-rt-t-250a-rt-p31772.html?&trstct=pol_12&nbc=1)
- DS18B20 [Link](https://www.reichelt.com/shelly-temperatur-sensor-ds18b20-shelly-ds18b20-p287127.html?&trstct=pos_1&nbc=1)
- J4 Connector 3-pin [Link](https://www.reichelt.de/de/en/3-pin-terminal-strip-spacing-5-08-akl-101-03-p36606.html?&nbc=1)
- J1, J3 Connector 2-pin [Link](https://www.reichelt.de/de/en/2-pin-terminal-strip-spacing-5-08-akl-101-02-p36605.html?&nbc=1)
- Housing [Link](https://www.reichelt.de/de/en/plastic-housing-series-1591-57-x-87-x-36-mm-black-1591xxlsbk-p221267.html?&nbc=1)



# Updates:
- 06.11.20: Version 1.3: Changed PCB layout to version 1.1. Larger terminal blocks and different transistor footprints for easy soldering.
- 01.11.20: Version 1.3: Added KiCAD files.
- 04.08.20: Version 1.3: Changed CAN_TX to 5. Added WLAN client mode. Store last Node Address.
- 24.07.20: Version 1.2: Fixed AP setting problem (sometimes 192.168.4.1 was set after reboot).
- 23.07.20: Version 1.1: Moved to new JSON library version 6.x.x.
- 23.07.20: Version 1.0: Changed task 0 priority to 0 (see Issues) and corrected TWA calculation.
- 11.12.19: Version 0.9: Added CAN pin definition in code. No need to change settings in include file of library.
- 17.10.19: Version 0.8. Increase JSON buffer improved ID calculation.
- 14.10.19: Version 0.7. Added JSON interface on port 90. Added unique ID.
- 16.09.19: Added GGA sentence in N2kDataToNMEA0183.cpp
- 03.08.19: Version 0.4. Added rudder, log information. Calculate TWS/TWD from apparent wind, heading and COG. Added web gauge for fridge temperature (web browser, port 80).
- 23.07.19: Version 0.2 fixes the full AIS message forwarding issue. I'm now using the GetMessage function from the MNEA0183 library.
