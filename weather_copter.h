//
//  weather_copter.h
//  
//
//  Created by Gregory C Lewin on 11/11/17.
//
//

#ifndef weather_copter_h
#define weather_copter_h

#include <trisonica.h>
#include <gps.h>
#include <SparkFunMPL3115A2.h>

struct WeatherDatum
{
    TrisonicaDatum triDatum;
    GPSdatum gpsDatam;
    MPL3115A2 altimeter;

    
    //TempPressDatum
    //OtherDatums?
};

class WeatherCopter
{
protected:
    GPS* gps = NULL;
    Trisonica* anemometer = NULL;
    Altimeter* altimeter = NULL;
    
    
    
public:
    WeatherCopter(void) {}
    
    bool CheckForNewDatum(void)
    {
        bool retVal = 0;
        
        if(gps->CheckSerial() & GGA)
        {
            //currGPSstring = gps->GetReading().MakeShortDataString();
//            WriteSD(filename, gps->GetReading().MakeShortDataString());
        }

        if(anemometer->CheckSerial())
        {
            //ReadTemperature();
            //ReadPressure();
//            WriteSD(filename, trisonica->GetReading().MakeDataString() + String(',') + currGPSstring);
        }
        
        return retVal;
    }
    
    String MakeDatumString(void)
    {
        String datum = gps->GetReading().MakeShortDataString() + anemometer->GetReading().MakeDataString();
        return datum;
    }
};

#endif /* weather_copter_h */
