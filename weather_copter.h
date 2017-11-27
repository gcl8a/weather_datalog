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
#include <MPL3115A2.h>

struct WeatherDatum
{
    TrisonicaDatum windDatum;
    GPSdatum gpsDatum;
    AltimeterDatum altDatum;

    //TempPressDatum
    //OtherDatums?
};

class WeatherCopter
{
protected:
    GPS* gps = NULL;
    Trisonica* anemometer = NULL;
    Altimeter* altimeter = NULL;
    
    WeatherDatum workingDatum;
public:
    WeatherCopter(void) {}
    
    bool CheckForNewDatums(void);
    
    String MakeDataString(void)
    {
        String datum = workingDatum.gpsDatum.MakeShortDataString() + workingDatum.windDatum.MakeDataString();
        return datum;
    }
};

#endif /* weather_copter_h */
