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
    
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    
    SerialUSB.println(F("Initializing SD card..."));
    
    // see if the card is present and can be initialized:
    if (!SD.begin(SD_CS, SPI_QUARTER_SPEED))
    {
        SerialUSB.println(F("Card failed."));
    }
    
    SerialUSB.println(F("Flash..."));
    flash.Init();
    
//    SerialUSB.println("Sizes:");
//    SerialUSB.println(sizeof(GPSDump));
//    SerialUSB.println(sizeof(AltimeterDump));
//    SerialUSB.println(sizeof(TrisonicaDump));
//    SerialUSB.println(sizeof(AHRSDump));
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

uint32_t WeatherCopter::ReadGPSAltDump(GPSDatum& gpsDatum, AltimeterDatum& altDatum)
{
    BufferArray buffer(sizeof(GPSDatum) + sizeof(AltimeterDatum));
    
    flash.ReadBytes(buffer);
    
    memcpy(&gpsDatum, &buffer[0], sizeof(GPSDatum));
    memcpy(&altDatum, &buffer[sizeof(GPSDatum)], sizeof(AltimeterDatum));
    
    return buffer.GetSize();
}

uint32_t WeatherCopter::ReadWindAndIMUDump(TrisonicaDatum& triDatum, AHRSDatum& imuDatum)
{
    BufferArray buffer(sizeof(TrisonicaDatum) + sizeof(AHRSDatum));
    
    flash.ReadBytes(buffer);
    
    memcpy( &triDatum, &buffer[0],sizeof(triDatum));
    memcpy(&imuDatum, &buffer[sizeof(triDatum)], sizeof(imuDatum));
    
    return buffer.GetSize();
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

uint32_t WeatherCopter::CloseStore(uint8_t storeNumber)
{
    return flash.CloseStore(storeNumber);
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
uint32_t WeatherCopter::SaveStoreToSD(uint16_t storeNumber)
{
    SerialUSB.print("Saving store ");
    SerialUSB.println(storeNumber);

    char filename[16];
    int test = 0;

    bool success = 0;
    while(!success)
    {
        sprintf(filename, "%s%02i%02i.csv", "test", storeNumber, test);
        SerialUSB.println(filename);
        if(!SD.exists(filename)) success = 1;
        else test++;
    }

    File dataFile = SD.open(filename, O_WRITE | O_CREAT | O_APPEND);
    if(!dataFile)
    {
        SerialUSB.println("Error opening file!");
        return 0;
    }

    uint32_t byteCount = 0;

    flash.Select(storeNumber);
    flash.RewindStore(storeNumber);
    
    TrisonicaDatum triDatum;
    AHRSDatum imuDatum;
    GPSDatum gpsDatum;
    AltimeterDatum altDatum;
    
    BufferArray index(1); //just need a byte, here
    
    flash.ReadBytes(index);
    while(index[0] != 0xff)
    {
        if(index[0]) //wind datum
        {
            byteCount += ReadWindAndIMUDump(triDatum, imuDatum);
            String dataStr = gpsDatum.MakeDataString() + ','
                + triDatum.MakeDataString() + ','
                + altDatum.MakeDataString() + ','
                + imuDatum.MakeDataString() + '\n';
            
            dataFile.print(dataStr);
            //SerialUSB.print(dataStr);
        }
        else
        {
            byteCount += ReadGPSAltDump(gpsDatum, altDatum);
        }
        
        flash.ReadBytes(index); //get next record
    }

    dataFile.close();

    SerialUSB.print("\nDone.");

    return byteCount;
}

uint32_t WeatherCopter::SplashStore(uint16_t storeNumber)
{
    SerialUSB.print("Splashing store ");
    SerialUSB.println(storeNumber);
    
    flash.RewindStore(storeNumber);
    
    uint32_t totalBytes = 0;
    
    //read 32 at a time to make printing "clean"...heh
    BufferArray buffer(32);
    uint32_t bytes = flash.ReadBytes(storeNumber, buffer);
    while(bytes)
    {
        SerialUSB.print(totalBytes);
        totalBytes += bytes;
        for(uint16_t i = 0; i < buffer.GetSize(); i++)
        {
            SerialUSB.print(',');
            SerialUSB.print(buffer[i], HEX);
        }
        SerialUSB.print('\n');
        bytes = flash.ReadBytes(storeNumber, buffer);
    }
    
    SerialUSB.print("Done ");
    
    return totalBytes;
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

