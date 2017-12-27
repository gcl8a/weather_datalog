//
//  weather_copter.h
//  
//
//  Created by Gregory C Lewin on 11/11/17.
//
//

//#ifndef weather_copter_h
//#define weather_copter_h

#pragma once

#include <trisonica.h>
#include <gps.h>
#include <MPL3115A2.h>
#include <BME280.h>
#include <LSM9DS1.h>

#define TRISONICA 0x10
#define ALT280 0x20
#define IMU 0x40

class WeatherCopter
{
protected:
    TrisonicaDatum windDatum;
    GPSdatum gpsDatum;
    AltimeterDatum altDatum3115;
    AltimeterDatum altDatum280;
    AHRSDatum ahrsDatum;

    Trisonica anemometer;
    GPS_MTK3339 gps;
    BME280 altimeter280;
    MPL3115A2 altimeter3115;
    LSM9DS1 imu;
    
public:
    WeatherCopter(HardwareSerial* triSer, HardwareSerial* gpsSer) : anemometer(triSer), gps(gpsSer)
    {}
    
    bool Init(void)
    {
        altimeter280.Init();
        imu.Init();
        gps.Init(); //doesn't really init; just sends data rate stuff

        imu.CalibrateMagnetometer(100);

        return 1;
    }
    
    uint8_t CheckForGPSDatum(void);
    bool CheckForWindDatum(void);
    bool CheckForIMUUpdate(void);
    bool CheckForAlt280Update(void);
    bool CheckForAlt3115Update(void);
    
    uint8_t CheckSensors(void);
    
    bool SetActiveNMEAStrings(uint8_t nmea) {return gps.SetActiveNMEAStrings(nmea);}
    int MakeFilename(void)
    {
        return gpsDatum.MakeMonthDay();
    }

    String MakeDataString(void)
    {
        String datum = gpsDatum.MakeShortDataString() + windDatum.MakeDataString()
                       + altDatum280.MakeDataString() + ahrsDatum.MakeDataString();
        return datum;
    }

    String MakeShortDataString(void)
    {
        String datum = gpsDatum.MakeShortDataString() + windDatum.MakeDataString();
        return datum;
    }
};

//#endif /* weather_copter_h */
