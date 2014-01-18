#include "utils.h"

boolean _strcmp( char *a, char* b, int l ) {
  for ( byte i=0; i < l; i++ ) {
    if ( a[i] != b[i] ) {
      return false;
    }
  }
  return true;
}

//TODO: consistency between these two
int fromDec(char* str, byte len) {
  int val = 0;
  for ( byte i=0; i < len; i++ ) {
    val = val * 10 + str[i] - '0';
  }
  return val;
}

float streamFloat(char* buff, int* strLen) {
  int i = 0;
  float value = 0.0;
  int decimal = 0;
  while( buff[++i] != ',' ) {
    if ( buff[i] == '.' ) {
      decimal = 10;   
    }
    else if ( decimal ) {
      value += (buff[i]-'0') / decimal;
      decimal = decimal * 10;
    }
    else {
      value *= 10;
      value += buff[i]-'0';
    }
  }
    
  i--;
  *strLen = i;
  return value;
}

