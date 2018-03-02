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
//#include <MPL3115A2.h>
#include <BME280.h>
#include <LSM9DS1.h>

#include <dataflash.h>
#include <SdFat.h>

#define FLASH_CS 7

/*
 * A collection of weather datasets (dump, pages, whatever). Essentially equivalent to a file
 * but since this is stored in flash, it's not really a "file"
 */

struct WeatherStore
{
    uint16_t storeNumber = 0;
    char fileName[16]; //8.3 + \0
};

class WeatherCopter
{
protected:
    FlashStoreManager flash;
    //Datastore* currStore;
    
    uint8_t windCount = 0;
public:
    WeatherCopter(void) : flash(new FlashAT45DB321E(&SPI, FLASH_CS)) {}
    
    bool Init(void);

    //for managing individual records
    uint32_t AddGPSAltDump(const GPSDatum&, const AltimeterDatum&);
    uint32_t AddWindAndIMUDump(const TrisonicaDatum&, const AHRSDatum&);
    uint32_t ReadGPSAltDump(GPSDatum& gpsDatum, AltimeterDatum& altDatum);
    uint32_t ReadWindAndIMUDump(TrisonicaDatum& triDatum, AHRSDatum& imuDatum);

    //for managing Flash storage
    void ListStores(bool refresh = false);
    uint32_t OpenStore(uint16_t);
    uint32_t EraseStore(uint16_t);
    uint32_t CloseStore(void);

    uint32_t SaveStoreToSD(uint16_t fileNumber);
    uint32_t SplashStore(uint16_t fileNumber);
};

#endif /* weather_copter_h */
