//
//  weather_copter.cpp
//
//
//  Created by Gregory C Lewin on 11/11/17.
//
//

#include <weather_copter.h>

#define DEFAULT_DATA_SIZE 524288ul  //~30 minutes of data at 10Hz

const int SD_CS = 9;
SdFat SD;

bool WeatherCopter::Init(void)
{
    SerialUSB.println("WC::Init()");
    
    //strncpy(filenameBase, fb, 5);
    //fb.substring(0, 4).toCharArray(filenameBase, 5); //max four characters + \0
    //SerialUSB.println(filenameBase);
    
//    SerialUSB.println(F("Initializing SD card..."));
//
//    pinMode(SD_CS, OUTPUT);
//    digitalWrite(SD_CS, HIGH);
//
//    // see if the card is present and can be initialized:
//    if (!SD.begin(SD_CS, SPI_QUARTER_SPEED))
//    {
//        SerialUSB.println(F("Card failed."));
//    }
    
//    SerialUSB.println("Sizes:");
//    SerialUSB.println(sizeof(GPSDump));
//    SerialUSB.println(sizeof(AltimeterDump));
//    SerialUSB.println(sizeof(TrisonicaDump));
//    SerialUSB.println(sizeof(AHRSDump));

    //open for writing, erase memory
    //IDdata id = flash.Init();
    flash.Init();
//    SerialUSB.println(flash.ReadStatus(), HEX);
//
//    delay(100);

    //move this elsewhere later...
    //flash.WriteEnable();
    //while(flash.IsBusy()) {}
    //flash.GlobalUnprotect();
    //SerialUSB.println(flash.ReadSectorProtectionStatus(0), HEX);
    
    SerialUSB.println("/WC::Init()");

    return true;
}

void WeatherCopter::AddGPSAltDump(const GPSDatum& gpsDatum, const AltimeterDatum& altDatum)
{
    BufferArray buffer(1 + sizeof(GPSDatum) + sizeof(AltimeterDatum));

    buffer[0] = 0; //indicates GPS (ie, 'new') record
    memcpy(&buffer[1], &gpsDatum, sizeof(GPSDatum));
    memcpy(&buffer[1 + sizeof(GPSDatum)], &altDatum, sizeof(AltimeterDatum));
    
    flash.Write(buffer);
    
    windCount = 0; //reset wind count
}

void WeatherCopter::AddWindAndIMUDump(const TrisonicaDatum& triDatum, const AHRSDatum& imuDatum)
{
    BufferArray buffer(1 + sizeof(TrisonicaDatum) + sizeof(AHRSDatum));
    
    buffer[0] = ++windCount; //tracks number of wind records for each GPS datum
    if(windCount == 255) windCount = 254; //if GPS fails, keep storing data but don't let count roll-over
    memcpy(&buffer[1], &triDatum, sizeof(triDatum));
    memcpy(&buffer[1 + sizeof(triDatum)], &imuDatum, sizeof(imuDatum));
    
    flash.Write(buffer);
}

void WeatherCopter::ListStores(bool refresh)
{
    TListIterator<Datastore> storeIterator = flash.GetStoresIterator(refresh);
    
    SerialUSB.println("Stores:");
    
    while(storeIterator.Current())
    {
        SerialUSB.print(storeIterator.Current()->Display());
        storeIterator++;
    }
}

uint32_t WeatherCopter::CreateStore(uint16_t number)
/*
 * Creates a new store with the next file number
 */
{
    return flash.CreateStore(number, DEFAULT_DATA_SIZE);
}

uint32_t WeatherCopter::EraseStore(uint8_t storeNumber)
{
    return flash.DeleteStore(storeNumber);
}

//Datastore* WeatherCopter::WriteCurrentDumpToFlash(void)
//{
//    currDump.buffer[1] = currDump.windDatums;
//    Datastore* datastore = flash.WritePageToCurrentStore(currDump.buffer);
//    if(!datastore)
//    {
//        SerialUSB.println(F("Error in WriteCurrentDumpToFlash()"));
//    }
//
//    return datastore; //upon success (needs checking), increment the endPage
//}
//
//uint16_t WeatherCopter::SaveStoreToSD(uint8_t storeNumber)
//{
//    SerialUSB.print("Saving store ");
//    SerialUSB.println(storeNumber);
//
//    char filename[16];
//    int test = 0;
//
//    bool success = 0;
//    while(!success)
//    {
//        sprintf(filename, "%s%02i%02i.csv", "test", storeNumber, test);
//        if(!SD.exists(filename)) success = 1;
//        else test++;
//    }
//
//    File dataFile = SD.open(filename, O_WRITE | O_CREAT | O_APPEND);
//    if(!dataFile)
//    {
//        SerialUSB.println("Error opening file!");
//        return 0;
//    }
//
//    //smarter in the future -- hard code file 1 for now...
//    uint16_t pageCount = 0;
//
//    BufferArray buffer = flash.ReadPageFromStore(storeNumber, true);
//    while(buffer.GetSize() != 0)
//    {
//        SerialUSB.println(++pageCount);
//
//        SaveBufferToSD(&dataFile, buffer);
//        dataFile.flush(); //check if this does what I think it does...
//
//        buffer = flash.ReadPageFromStore(storeNumber);
//    }
//
//    dataFile.close();
//
//    SerialUSB.print("\nDone.");
//
//    return pageCount;
//}

uint16_t WeatherCopter::SplashStore(uint8_t storeNumber)
{
    SerialUSB.print("Splashing store ");
    SerialUSB.println(storeNumber);
    
    uint16_t pageCount = 0;
    
    BufferArray buffer;// = flash.ReadPageFromStore(storeNumber, true);
    while(buffer.GetSize() != 0)
    {
        SerialUSB.print(++pageCount);
        for(uint16_t i = 0; i < buffer.GetSize(); i++)
        {
            SerialUSB.print(',');
            SerialUSB.print(buffer[i], HEX);
        }
        SerialUSB.print('\n');
        //buffer = flash.ReadPageFromStore(storeNumber);
    }
    
    SerialUSB.print("Done ");
    
    return pageCount;
}

//uint8_t WeatherCopter::SaveBufferToSD(File* dataFile, const BufferArray& buffer)
//{
//    uint8_t windDatums = buffer[1];
//
//    //GPSDump gpsDump;
//
//    GPSDatum gpsDatum;
//    memcpy(&gpsDatum, &buffer[2], sizeof(GPSDatum));
//    String gpsStr = gpsDatum.MakeDataString();
//
//    AltimeterDump altDump;
//    memcpy(&altDump, &buffer[22], 8);
//
//    AltimeterDatum altDatum;
//    String altStr = String(',') + altDatum.ParseDataDump(altDump);
//
//    for(int i = 0; i < windDatums; i++)
//    {
//        TrisonicaDatum triDatum;
//        memcpy(&triDatum, &buffer[30 + i * 18], 12);
//        String triStr = ',' + triDatum.MakeDataString();
//
//        AHRSDatum imuDatum;
//        memcpy(&imuDatum, &buffer[30 + i * 18 + 12], 6);
//        String imuStr = ',' + imuDatum.MakeDataString();
//
//        String dataString = gpsStr + altStr + triStr + imuStr;
//
//        int lenStr = dataString.length();
//        if(lenStr > 127) dataString = dataString.substring(0, 128);
//        for(int i = lenStr + 1; i <= 127; i++) //get to 127
//        {
//            dataString += ' ';
//        }
//        dataString += '\n'; //then add \n to make it 128
//
//        //SerialUSB.println(dataString);
//
//        dataFile->print(dataString);
//    }
//
//    return windDatums;
//}
//
//

//uint8_t WeatherCopter::CheckForGPSDatum(void)
//{
//    bool retVal = 0;
//
//    GPSdatum datum;
//    if(gps.CheckSerial(&datum) == GGA)
//    {
//        gpsDatum = datum;
//        retVal = gpsDatum.source;
//    }
//
//    return retVal;
//}
//
//bool WeatherCopter::CheckForWindDatum(void)
//{
//    bool retVal = false;
//
//    TrisonicaDatum newDatum;
//    if(anemometer.CheckSerial(&newDatum))
//    {
//        windDatum = newDatum;
//        retVal = true;
//    }
//
//    return retVal;
//}
//
//bool WeatherCopter::CheckForIMUUpdate(void)
//{
//    bool retVal = false;
//
//    AHRSDatum newDatum;
//    if(imu.CheckInput(&newDatum))
//    {
//        ahrsDatum = newDatum;
//        retVal = true;
//    }
//
//    return retVal;
//}
//
//bool WeatherCopter::CheckForAlt280Update(void)
//{
//    bool retVal = false;
//
//    AltimeterDatum newDatum;
//    if(altimeter280.CheckForNewDatum(newDatum))
//    {
//        altDatum280 = newDatum;
//        retVal = true;
//    }
//
//    return retVal;
//}
//
//bool WeatherCopter::CheckForAlt3115Update(void)
//{
//    bool retVal = false;
//
//    AltimeterDatum newDatum;
//    if(altimeter3115.CheckForNewDatum(newDatum))
//    {
//        altDatum3115 = newDatum;
//        retVal = true;
//    }
//
//    return retVal;
//}
//
//uint8_t WeatherCopter::CheckSensors(void)
//{
//    uint8_t retVal = 0;
//    if(CheckForGPSDatum() == GGA) //only need GGA for the weather copter
//    {
//        retVal |= GGA;
//    }
//
//    if(CheckForWindDatum())
//    {
//        retVal |= TRISONICA;
//    }
//
//    if(CheckForAlt280Update())
//    {
//        retVal |= ALT280;
//    }
//
//    if(CheckForIMUUpdate())
//    {
//        retVal |= IMU;
//    }
//
//    return retVal;
//}

//uint8_t WeatherCopter::CloseStore(void)
//{
//    //write a basic index to the first page
//    uint8_t buffer[5];
//    buffer[0] = currStore.fileNumber;
//
//    memcpy(&buffer[1], &currStore.startPage, 2);
//    memcpy(&buffer[3],   &currStore.endPage, 2);
//
//    //flash.WritePage(5 * (currStore.fileNumber - 1), buffer, 5);
//
//    return currStore.fileNumber;
//}

