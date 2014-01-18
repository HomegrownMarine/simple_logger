
#include <SPI.h>
#include <SD.h>
#include <Time.h>
#include <PString.h>
#include <Streaming.h>

#include "utils.h"

#include "NMEAReader.h"

#define SD_B_S 512
#define LED_PIN 11
#define SPI_SS_PIN 0

// should only contain complete sentances
char sdBuffer[SD_B_S];
int sdWritePos = 0;
int sdReadPos = 0;

boolean writeToFile = false;

NMEAReader GPSReader(&Serial1, gpsSentenceHandler);

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(LED_PIN, OUTPUT);     
  digitalWrite(LED_PIN, LOW);
  
  Serial.begin(9600);
  Serial1.begin(4800);
  
  pinMode(SPI_SS_PIN, OUTPUT);
  if ( !SD.begin(SPI_SS_PIN) ) {
//    //no logging, display this somehow.
    Serial.println("ERROR: failed to initialize SD writing.");
  }
  
  Serial.println("Running!!!");
}

void loop() {
    byte gpsStatus = GPSReader.read( );

    digitalWrite(LED_PIN, gpsStatus==0?LOW:HIGH);

    // try and wait for GPS to get a fix, so we write
    // into file with correct time.  After 5 sec, write anyways.
    // todo: make same as time set
    if ( gpsStatus != 0 || millis() > 60000 ) {
      writeToFile = true;
    }

    writeLog();
}

unsigned long lastGPSFix = 0;

void gpsSentenceHandler( char* buff, byte len ) {
  // $--RMC,hhmmss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,ddmmyy,x.x,a*hh
  // $GPRMC,180010,A,4741.7508,N,12225.7919,W,004.5,165.3,210412,018.2,E*6F

  if ( _strcmp("RMC", buff+3, 3) ) {
    lastGPSFix = millis();
    
    byte hr, mn, sec, day, month, yr;
    //have to find comas, etc
    byte field = 0;
    for ( byte i=0; i < len; i++ ) {
      if ( buff[i] == ',' ) {
        field++;
        if ( field == 1 ) {
          hr = fromDec(buff+i+1,2);
          mn = fromDec(buff+i+3,2);
          sec = fromDec(buff+i+5,2);
          i += 6;
        }
        else if ( field == 9 ) {
          day = fromDec(buff+i+1,2);
          month = fromDec(buff+i+3,2);
          yr = fromDec(buff+i+5,2);
          break;
        }
      }
    }

    // every 256 sentences, or so, parse the rmc message to sync up the system time
    // TODO: commment out time syncing in time library
//      Serial << "Setting time: " << hr << ":" << mn << ":" << sec << "  " << day << "-" << month << "-" << yr << endl;
    setTime(hr,mn,sec,day,month,yr);
  }

  processSentence(buff, len);
}

void processSentence(char* buff, byte len) {
  int startPos = sdWritePos;
  for ( byte i=0; i < len; i++, sdWritePos = (sdWritePos+1) % SD_B_S ) {
    sdBuffer[sdWritePos] = buff[i];  
  }
}


char filename[] = "00000000.txt";
byte lastWriteHour = 100; //unreal hour, never set filename

char * getFilename() {
  time_t t = now();
  byte hr = hour(t);
  
  if ( hr != lastWriteHour ) {
    lastWriteHour = hr;

    //hr
    byte n = year(t) % 100;
    filename[0] = ( n / 10 ) + '0';
    filename[1] = ( n % 10 ) + '0';
    
    //month
    n = month(t);
    filename[2] = ( n / 10 ) + '0';  
    filename[3] = ( n % 10 ) + '0';

    //day
    n = day(t);
    filename[4] = ( n / 10 ) + '0';
    filename[5] = ( n % 10 ) + '0';
    
    //hour
    filename[6] = ( hr / 10 ) + '0';  
    filename[7] = ( hr % 10 ) + '0';
  }
  
  return filename;
}

void writeLog() {
  if ( !writeToFile )
    return;
  
  if ( sdWritePos != sdReadPos ) {
    
    //ii open the file. note that only one file can be open at a time,
    //ii so you have to close this one before opening another.
    File dataFile = SD.open(getFilename(), FILE_WRITE);

    if (dataFile) {
      if ( sdReadPos < sdWritePos ) {
        dataFile.write((byte*)sdBuffer+sdReadPos, sdWritePos-sdReadPos);
      }
      else {
        dataFile.write((byte*)sdBuffer+sdReadPos, SD_B_S-sdReadPos);
        dataFile.write((byte*)sdBuffer, sdWritePos);
      }

      dataFile.close();
  
      sdReadPos = sdWritePos;
    }
  }
}
