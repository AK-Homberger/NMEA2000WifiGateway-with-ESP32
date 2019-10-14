/*
  N2kDataToNMEA0183.cpp

  Copyright (c) 2015-2018 Timo Lappalainen, Kave Oy, www.kave.fi

  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
  Software, and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
  CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
  OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <N2kMessages.h>
#include <NMEA0183Messages.h>
#include <math.h>

#include "N2kDataToNMEA0183.h"
#include "BoatData.h"

const double radToDeg = 180.0 / M_PI;

//*****************************************************************************
void tN2kDataToNMEA0183::HandleMsg(const tN2kMsg &N2kMsg) {
  switch (N2kMsg.PGN) {
    case 127250UL: HandleHeading(N2kMsg);
    case 127258UL: HandleVariation(N2kMsg);
    case 128259UL: HandleBoatSpeed(N2kMsg);
    case 128267UL: HandleDepth(N2kMsg);
    case 129025UL: HandlePosition(N2kMsg);
    case 129026UL: HandleCOGSOG(N2kMsg);
    case 129029UL: HandleGNSS(N2kMsg);
    case 130306UL: HandleWind(N2kMsg);
    case 128275UL: HandleLog(N2kMsg);
    case 127245UL: HandleRudder(N2kMsg);
    case 130310UL: HandleWaterTemp(N2kMsg);
  }
}

//*****************************************************************************
long tN2kDataToNMEA0183::Update(tBoatData *BoatData) {
  SendRMC();
  if ( LastHeadingTime + 2000 < millis() ) Heading = N2kDoubleNA;
  if ( LastCOGSOGTime + 2000 < millis() ) {
    COG = N2kDoubleNA;
    SOG = N2kDoubleNA;
  }
  if ( LastPositionTime + 4000 < millis() ) {
    Latitude = N2kDoubleNA;
    Longitude = N2kDoubleNA;
  }
  if ( LastWindTime + 2000 < millis() ) {
    AWS = N2kDoubleNA;
    AWA = N2kDoubleNA;
    TWS = N2kDoubleNA;
    TWA = N2kDoubleNA;
    TWD = N2kDoubleNA;    
  }

  BoatData->Latitude=Latitude;
  BoatData->Longitude=Longitude;
  BoatData->Altitude=Altitude;
  BoatData->Heading=Heading * radToDeg;
  BoatData->COG=COG * radToDeg;
  BoatData->SOG=SOG * radToDeg;
  BoatData->STW=STW * 3600.0/1852.0;;
  BoatData->AWS=AWS * radToDeg;
  BoatData->TWS=TWS * 3600.0/1852.0;;
  BoatData->MaxAws=MaxAws * 3600.0/1852.0;;
  BoatData->MaxTws=MaxTws * 3600.0/1852.0;;
  BoatData->AWA=AWA * radToDeg;
  BoatData->TWA=TWA * radToDeg;
  BoatData->TWD=TWD * radToDeg;
  
  BoatData->TripLog=TripLog / 1825.0;
  BoatData->Log=Log / 1825.0;
  BoatData->RudderPosition=RudderPosition * radToDeg;
  BoatData->WaterTemperature=KelvinToC(WaterTemperature) ;
  BoatData->WaterDepth=WaterDepth;
  BoatData->Variation=Variation *radToDeg;
  BoatData->GPSTime=SecondsSinceMidnight;
  BoatData->DaysSince1970=DaysSince1970;
    
  
if (SecondsSinceMidnight!=N2kDoubleNA && DaysSince1970!=N2kUInt16NA){
   return((DaysSince1970*3600*24)+SecondsSinceMidnight); // Needed for SD Filename and time
  } else {
   return(0); // Needed for SD Filename and time 
  }
}

//*****************************************************************************
void tN2kDataToNMEA0183::SendMessage(const tNMEA0183Msg &NMEA0183Msg) {
  if ( pNMEA0183 != 0 ) pNMEA0183->SendMessage(NMEA0183Msg);
  if ( SendNMEA0183MessageCallback != 0 ) SendNMEA0183MessageCallback(NMEA0183Msg);
}

//*****************************************************************************
void tN2kDataToNMEA0183::HandleHeading(const tN2kMsg &N2kMsg) {
  unsigned char SID;
  tN2kHeadingReference ref;
  double _Deviation = 0;
  double _Variation;
  tNMEA0183Msg NMEA0183Msg;

  if ( ParseN2kHeading(N2kMsg, SID, Heading, _Deviation, _Variation, ref) ) {
    if ( ref == N2khr_magnetic ) {
      if ( !N2kIsNA(_Variation) ) Variation = _Variation; // Update Variation
      if ( !N2kIsNA(Heading) && !N2kIsNA(Variation) ) Heading -= Variation;
    }
    LastHeadingTime = millis();
    if ( NMEA0183SetHDG(NMEA0183Msg, Heading, _Deviation, Variation) ) {
      SendMessage(NMEA0183Msg);
    }
  }
}

//*****************************************************************************
void tN2kDataToNMEA0183::HandleVariation(const tN2kMsg &N2kMsg) {
  unsigned char SID;
  tN2kMagneticVariation Source;
  uint16_t DaysSince1970;
  
  ParseN2kMagneticVariation(N2kMsg, SID, Source, DaysSince1970, Variation);
}

//*****************************************************************************
void tN2kDataToNMEA0183::HandleBoatSpeed(const tN2kMsg &N2kMsg) {
  unsigned char SID;
  double WaterReferenced;
  double GroundReferenced;
  tN2kSpeedWaterReferenceType SWRT;

  if ( ParseN2kBoatSpeed(N2kMsg, SID, WaterReferenced, GroundReferenced, SWRT) ) {
    tNMEA0183Msg NMEA0183Msg;
    STW=WaterReferenced;
    double MagneticHeading = ( !N2kIsNA(Heading) && !N2kIsNA(Variation) ? Heading + Variation : NMEA0183DoubleNA);
    if ( NMEA0183SetVHW(NMEA0183Msg, Heading, MagneticHeading, WaterReferenced) ) {
      SendMessage(NMEA0183Msg);
    }
  }
}

//*****************************************************************************
void tN2kDataToNMEA0183::HandleDepth(const tN2kMsg &N2kMsg) {
  unsigned char SID;
  double DepthBelowTransducer;
  double Offset;
  double Range;

  if ( ParseN2kWaterDepth(N2kMsg, SID, DepthBelowTransducer, Offset, Range) ) {
    
    WaterDepth=DepthBelowTransducer+Offset;
    
    tNMEA0183Msg NMEA0183Msg;
    if ( NMEA0183SetDPT(NMEA0183Msg, DepthBelowTransducer, Offset) ) {
      SendMessage(NMEA0183Msg);
    }
    if ( NMEA0183SetDBx(NMEA0183Msg, DepthBelowTransducer, Offset) ) {
      SendMessage(NMEA0183Msg);
    }
  }
}

//*****************************************************************************
void tN2kDataToNMEA0183::HandlePosition(const tN2kMsg &N2kMsg) {

  if ( ParseN2kPGN129025(N2kMsg, Latitude, Longitude) ) {
    LastPositionTime = millis();
  }
}

//*****************************************************************************
void tN2kDataToNMEA0183::HandleCOGSOG(const tN2kMsg &N2kMsg) {
  unsigned char SID;
  tN2kHeadingReference HeadingReference;
  tNMEA0183Msg NMEA0183Msg;

  if ( ParseN2kCOGSOGRapid(N2kMsg, SID, HeadingReference, COG, SOG) ) {
    LastCOGSOGTime = millis();
    double MCOG = ( !N2kIsNA(COG) && !N2kIsNA(Variation) ? COG - Variation : NMEA0183DoubleNA );
    if ( HeadingReference == N2khr_magnetic ) {
      MCOG = COG;
      if ( !N2kIsNA(Variation) ) COG -= Variation;
    }
    if ( NMEA0183SetVTG(NMEA0183Msg, COG, MCOG, SOG) ) {
      SendMessage(NMEA0183Msg);
    }
  }
}

//*****************************************************************************
void tN2kDataToNMEA0183::HandleGNSS(const tN2kMsg &N2kMsg) {
  unsigned char SID;
  tN2kGNSStype GNSStype;
  tN2kGNSSmethod GNSSmethod;
  unsigned char nSatellites;
  double HDOP;
  double PDOP;
  double GeoidalSeparation;
  unsigned char nReferenceStations;
  tN2kGNSStype ReferenceStationType;
  uint16_t ReferenceSationID;
  double AgeOfCorrection;

  if ( ParseN2kGNSS(N2kMsg, SID, DaysSince1970, SecondsSinceMidnight, Latitude, Longitude, Altitude, GNSStype, GNSSmethod,
                    nSatellites, HDOP, PDOP, GeoidalSeparation,
                    nReferenceStations, ReferenceStationType, ReferenceSationID, AgeOfCorrection) ) {
    LastPositionTime = millis();
  }
}

//*****************************************************************************
void tN2kDataToNMEA0183::HandleWind(const tN2kMsg &N2kMsg) {
  unsigned char SID;
  tN2kWindReference WindReference;
  tNMEA0183WindReference NMEA0183Reference = NMEA0183Wind_True;

  double x, y;
  double WindAngle, WindSpeed;
  
  // double TWS, TWA, AWD;
  
  if ( ParseN2kWindSpeed(N2kMsg, SID, WindSpeed, WindAngle, WindReference) ) {
    tNMEA0183Msg NMEA0183Msg;
    LastWindTime = millis();
    
    if ( WindReference == N2kWind_Apparent ) {
        NMEA0183Reference = NMEA0183Wind_Apparent;
        AWA=WindAngle;
        AWS=WindSpeed;
        if (AWS > MaxAws) MaxAws=AWS;
       }

    if ( NMEA0183SetMWV(NMEA0183Msg, WindAngle, NMEA0183Reference , WindSpeed)) SendMessage(NMEA0183Msg);

    if (WindReference == N2kWind_Apparent && SOG != N2kDoubleNA) { // Lets calculate and send TWS/TWA if SOG is available
      
      AWD=WindAngle*radToDeg + Heading*radToDeg;
      if (AWD>360) AWD=AWD-360;
      if (AWD<0) AWD=AWD+360;

      x = WindSpeed * cos(WindAngle);
      y = WindSpeed * sin(WindAngle);

      TWA = atan2(y, -SOG + x);
      TWS = sqrt(( y*y) + ((-SOG+x)*(-SOG+x)));

      if (TWS > MaxTws) MaxTws=TWS;

      TWA = TWA * radToDeg +360;

      if (TWA>360) TWA=TWA-360;
      if (TWA<0) TWA=TWA+360;
      
      NMEA0183Reference = NMEA0183Wind_True;
      if ( NMEA0183SetMWV(NMEA0183Msg, TWA, NMEA0183Reference , TWS)) SendMessage(NMEA0183Msg);
      
      if ( !NMEA0183Msg.Init("MWD", "GP") ) return;
      if ( !NMEA0183Msg.AddDoubleField(AWD) ) return;
      if ( !NMEA0183Msg.AddStrField("T") ) return;
      if ( !NMEA0183Msg.AddDoubleField(AWD) ) return;
      if ( !NMEA0183Msg.AddStrField("M") ) return;
      if ( !NMEA0183Msg.AddDoubleField(TWS/0.514444) ) return;
      if ( !NMEA0183Msg.AddStrField("N") ) return;
      if ( !NMEA0183Msg.AddDoubleField(TWS) ) return;
      if ( !NMEA0183Msg.AddStrField("M") ) return;

      SendMessage(NMEA0183Msg);

     TWA=TWA/radToDeg;
     
     }
  }
}
  //*****************************************************************************
  void tN2kDataToNMEA0183::SendRMC() {
    if ( NextRMCSend <= millis() && !N2kIsNA(Latitude) ) {
      tNMEA0183Msg NMEA0183Msg;
      if ( NMEA0183SetRMC(NMEA0183Msg, SecondsSinceMidnight, Latitude, Longitude, COG, SOG, DaysSince1970, Variation) ) {
        SendMessage(NMEA0183Msg);
      }
      SetNextRMCSend();
    }
  }


  //*****************************************************************************
  void tN2kDataToNMEA0183::HandleLog(const tN2kMsg & N2kMsg) {
  uint16_t DaysSince1970;
  double SecondsSinceMidnight;

    if ( ParseN2kDistanceLog(N2kMsg, DaysSince1970, SecondsSinceMidnight, Log, TripLog) ) {

      tNMEA0183Msg NMEA0183Msg;

      if ( !NMEA0183Msg.Init("VLW", "GP") ) return;
      if ( !NMEA0183Msg.AddDoubleField(Log / 1852.0) ) return;
      if ( !NMEA0183Msg.AddStrField("N") ) return;
      if ( !NMEA0183Msg.AddDoubleField(TripLog / 1852.0) ) return;
      if ( !NMEA0183Msg.AddStrField("N") ) return;

      SendMessage(NMEA0183Msg);
    }
  }

  //*****************************************************************************
  void tN2kDataToNMEA0183::HandleRudder(const tN2kMsg & N2kMsg) {

    unsigned char Instance;
    tN2kRudderDirectionOrder RudderDirectionOrder;
    double AngleOrder;

    if ( ParseN2kRudder(N2kMsg, RudderPosition, Instance, RudderDirectionOrder, AngleOrder) ) {

      if(Instance!=0) return;

      tNMEA0183Msg NMEA0183Msg;

      if ( !NMEA0183Msg.Init("RSA", "GP") ) return;
      if ( !NMEA0183Msg.AddDoubleField(RudderPosition * radToDeg) ) return;
      if ( !NMEA0183Msg.AddStrField("A") ) return;
      if ( !NMEA0183Msg.AddDoubleField(0.0) ) return;
      if ( !NMEA0183Msg.AddStrField("A") ) return;

      SendMessage(NMEA0183Msg);
    }
  }

//*****************************************************************************
  void tN2kDataToNMEA0183::HandleWaterTemp(const tN2kMsg & N2kMsg) {

    unsigned char SID;
    double OutsideAmbientAirTemperature;
    double AtmosphericPressure;

    if ( ParseN2kPGN130310(N2kMsg, SID, WaterTemperature, OutsideAmbientAirTemperature, AtmosphericPressure) ) {

      tNMEA0183Msg NMEA0183Msg;

      if ( !NMEA0183Msg.Init("MTW", "GP") ) return;
      if ( !NMEA0183Msg.AddDoubleField(KelvinToC(WaterTemperature))) return;
      if ( !NMEA0183Msg.AddStrField("C") ) return;
      
      SendMessage(NMEA0183Msg);
    }
  }
