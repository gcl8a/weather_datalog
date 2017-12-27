//
//  weather_copter.cpp
//  
//
//  Created by Gregory C Lewin on 11/11/17.
//
//

#include <weather_copter.h>


uint8_t WeatherCopter::CheckForGPSDatum(void)
{
    bool retVal = 0;
    
    GPSdatum datum;
    if(gps.CheckSerial(&datum) == GGA)
    {
        gpsDatum = datum;
        retVal = gpsDatum.source;
    }
    
    return retVal;
}

bool WeatherCopter::CheckForWindDatum(void)
{
    bool retVal = false;
    
    TrisonicaDatum newDatum;
    if(anemometer.CheckSerial(&newDatum))
    {
        windDatum = newDatum;
        retVal = true;
    }
    
    return retVal;
}

bool WeatherCopter::CheckForIMUUpdate(void)
{
    bool retVal = false;
    
    AHRSDatum newDatum;
    if(imu.CheckInput(&newDatum))
    {
        ahrsDatum = newDatum;
        retVal = true;
    }
    
    return retVal;
}

bool WeatherCopter::CheckForAlt280Update(void)
{
    bool retVal = false;
    
    AltimeterDatum newDatum;
    if(altimeter280.CheckForNewDatum(newDatum))
    {
        altDatum280 = newDatum;
        retVal = true;
    }
    
    return retVal;
}

bool WeatherCopter::CheckForAlt3115Update(void)
{
    bool retVal = false;
    
    AltimeterDatum newDatum;
    if(altimeter3115.CheckForNewDatum(newDatum))
    {
        altDatum3115 = newDatum;
        retVal = true;
    }
    
    return retVal;
}

uint8_t WeatherCopter::CheckSensors(void)
{
    uint8_t retVal = 0;
    if(CheckForGPSDatum() == GGA) //only need GGA for the weather copter
    {
        retVal |= GGA;
    }

    if(CheckForWindDatum())
    {
        retVal |= TRISONICA;
    }

    if(CheckForAlt280Update())
    {
        retVal |= ALT280;
    }

    if(CheckForIMUUpdate())
    {
        retVal |= IMU;
    }
    
    return retVal;
}
