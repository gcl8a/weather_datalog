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
//#include <TList.h>

#include <SdFat.h>

#define FLASH_CS 7

/*
 * A collection of weather datasets (dump, pages, whatever). Essentially equivalent to a file
 * but since this is stored in flash, it's not really a "file"
 */

struct WeatherStore
{
    uint16_t fileNumber = 0;
    char fileName[16]; //8.3 + \0
};

class WeatherCopter
{
protected:
    FlashStoreManager flash;
    WeatherStore currStore;
    
    uint8_t windCount = 0;
public:
    WeatherCopter(void) : flash(new FlashAT45DB321E(&SPI, FLASH_CS)) {}
    
    bool Init(void);

    //for managing individual records
    //uint8_t CreateRecord(void);
    void AddGPSAltDump(const GPSDatum&, const AltimeterDatum&);
    void AddWindAndIMUDump(const TrisonicaDatum&, const AHRSDatum&);
    
    //for managing Flash storage
    void ListStores(bool refresh = true);
    uint32_t CreateStore(uint16_t);
    uint32_t EraseStore(uint8_t fileNumber);

//    Datastore* WriteCurrentDumpToFlash(void);
    uint16_t SaveStoreToSD(uint8_t fileNumber);
    uint16_t SplashStore(uint8_t fileNumber);

protected:
    //for managing Flash storage
    //uint8_t SaveBufferToSD(File* dataFile, const BufferArray& buffer);
};

#endif /* weather_copter_h */
