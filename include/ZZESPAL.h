#ifndef ZZESPAL_H
#define ZZESPAL_H

#include <Arduino.h>

#ifndef ESPAL_ID_ENCODING_BASE
  #define ESPAL_ID_ENCODING_BASE 16
#endif

#ifndef ESPAL_SHORT_ID_LENGTH
  #define ESPAL_SHORT_ID_LENGTH 4
#endif

class HalClass
{

public:

  virtual uint64_t getChipId( void ) = 0;
  virtual uint32_t getFreeHeap( void ) = 0;
  virtual uint32_t getFlashChipSize( void ) = 0;
  virtual size_t getUpdateSize( void ) = 0;

  virtual void reset( void ) = 0;
  virtual void eraseConfig( void ) = 0;
  virtual String getChipInfo( void ) = 0;

  virtual String getShortId( void );
  virtual String getLongId( int base = ESPAL_ID_ENCODING_BASE );
  virtual void begin( void );
};

#include "ZZESPAL_esp32.h"

extern HalEsp32 ESPAL;

#endif /* ESPAL_H */ 
