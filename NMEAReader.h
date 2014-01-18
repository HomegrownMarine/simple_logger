

#ifndef NMEAReader_h
#define NMEAReader_h

#include "Arduino.h"

typedef void(*sentenceHandler)(char* buff, byte len);

#define NMEA_READ_BUFFER 96

class NMEAReader {
  
  char buffer[NMEA_READ_BUFFER];
  byte bufferPos;
  byte checksum; 
  
  unsigned long lastDataTime;
  unsigned long lastSentenceTime;
  
  unsigned int sentencesReceived;
  unsigned int checksumsFailed;
  
  Stream *serialPort;
  
  sentenceHandler newSentenceHandler;
  
  public:
    NMEAReader( Stream *port, sentenceHandler handler );

    byte read();
    
    unsigned long getSentencesReceived() { return sentencesReceived; }
    void clearSentencesReceived() { sentencesReceived = 0; }
};

#endif
