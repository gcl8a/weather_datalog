//
//  weather_copter.cpp
//  
//
//  Created by Gregory C Lewin on 11/11/17.
//
//

#include <weather_copter.h>

bool WeatherCopter::CheckForNewDatums(void)
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

