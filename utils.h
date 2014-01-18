#ifndef utils_h
#define utils_h

#include "Arduino.h"

boolean _strcmp( char *a, char* b, int l );
int fromDec(char* str, byte len);
float streamFloat(char* buff, int* strLen);
#endif
