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
 * One set of weather copter data, equivalent to one page of data
 * Includes a GPS and Altimeter stamp + up to 11 Trisonica + IMU stamps
 */

struct WeatherDump
{
    uint8_t fileNumber = 0;
    uint8_t windDatums = 0; //number of Trisonica/IMU readings in this record (max 11; 255 indicates error)
    
    BufferArray buffer;
    //uint8_t bufferLength = 0; //current byte
    
    WeatherDump(void) : buffer(256) {}  //hard-coded for now...
};

/*
 * A collection of weather datasets (dump, pages, whatever). Essentially equivalent to a file
 * but since this is stored in flash, it's not really a "file"
 */

struct WeatherStore
{
    uint8_t fileNumber = 0;
    char fileName[8];
    
    uint16_t startPage = 1;
    uint16_t endPage = 0;
};

class WeatherCopter
{
protected:
    WeatherDump currDump;   //the current dataset
    char filenameBase[8];

    Flashstore flash;
public:
    WeatherCopter(void) : flash(&SPI, FLASH_CS) {}
    
    bool Init(const char*);

    //for managing individual records
    uint8_t CreateRecord(void);
    void AddGPSDump(const GPSDatum&);
    void AddAltDump(const AltimeterDump& altDump);
    void AddWindAndIMUDump(const TrisonicaDatum& triDatum, const AHRSDatum& imuDatum);
    
    //for managing Flash storage
    void ListStores(bool refresh = true);
    Datastore* CreateStore(uint8_t);
    uint16_t EraseStore(uint8_t fileNumber);

    Datastore* WriteCurrentDumpToFlash(void);
    uint16_t SaveStoreToSD(uint8_t fileNumber);
    
    uint16_t SplashStore(uint8_t fileNumber);

protected:
    //for managing Flash storage
    uint8_t SaveBufferToSD(File* dataFile, const BufferArray& buffer);
};

#endif /* weather_copter_h */
