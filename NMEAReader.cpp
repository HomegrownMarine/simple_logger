/*
Copyright (c) 2012, Grady Morgan
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "NMEAReader.h"
#include <Time.h>

int fromHex(char* str, byte len);

NMEAReader::NMEAReader( Stream *port, sentenceHandler handler ) {
  bufferPos = 0;
  checksum = 0; 
  
  sentencesReceived = 0;
  checksumsFailed = 0;
  
  lastDataTime = 0;
  lastSentenceTime = 0;
  
  serialPort = port;
  newSentenceHandler = handler;
}

byte NMEAReader::read() {
  byte read = 0;
  unsigned long now_t = millis();
  while( serialPort->available() ) {
    lastDataTime = now_t;
    

    
    char nmeaChar = serialPort->read();
    Serial.print(nmeaChar);
    
    checksum ^= nmeaChar;
    buffer[bufferPos] = nmeaChar;
    bufferPos = bufferPos+1;
    //TODO: checksum

    if ( bufferPos >= NMEA_READ_BUFFER ) {
//      Serial.println("ERROR: buffer overflow");
      bufferPos = 0;
      checksum = 0;
    }
    else if ( nmeaChar == '\n' ) {
      lastSentenceTime = now_t;
      
      //TODO: loose \r\n, to save buffer room
      
      //back out $ and checksum
      checksum ^= buffer[0];
      checksum ^= buffer[bufferPos-1];  //\n
      checksum ^= buffer[bufferPos-2];  //\r
      checksum ^= buffer[bufferPos-3];  //F
      checksum ^= buffer[bufferPos-4];  //F
      checksum ^= buffer[bufferPos-5];  //*
      
      if ( fromHex(buffer+bufferPos-4, 2) != checksum ) {
//        Serial.print("Checksum error - ");
//        Serial.print(fromHex(buffer+bufferPos-4, 2));
//        
//        Serial.print('\t');
//        Serial.print(checksum);
//        Serial.println();
//
          checksumsFailed++;
      }
      else { 

//        Serial.print(now());
//        Serial.print('\t');
//        Serial.write((byte*)buffer, bufferPos);
        
        newSentenceHandler(buffer, bufferPos);
        sentencesReceived++;
      }
      
      bufferPos=0;
      checksum=0;
    }
  }
  
  if ( now_t - lastSentenceTime < 1000 ) {
    read = 3;
  }
  else if ( now_t - lastSentenceTime < 5000 ) {
    read = 2;
  }
  else if ( now_t - lastDataTime < 5000 ) 
    read = 1;

  return read;
}

int fromHex(char* str, byte len) {
  int val = 0;
  for ( byte i=0; i < len; i++ ) {
    val = val * 16;
    char a = str[i];
    if (str[i] >= 'A' && str[i] <= 'F')
      val += str[i] - 'A' + 10;
    //    else if (str[i] >= 'a' && str[i] <= 'f')
    //      val += str[i] - 'a' + 10;
    else
      val += str[i] - '0';
  }
  return val; 
}
