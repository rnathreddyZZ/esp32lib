
#ifndef ZZESPAL_H
  #error Do not include directly, #include "AXESPAL.h"
#endif // !_ESPAL_H

class HalEsp32 : public HalClass
{

public:

  virtual uint64_t getChipId( void )
  {
    return ESP.getEfuseMac( );
  }

  virtual uint32_t getFreeHeap( void ) 
  {
    return ESP.getFreeHeap();
  }

  virtual uint32_t getFlashChipSize( void ) 
  {
    return ESP.getFlashChipSize();
  }

  virtual size_t getUpdateSize( void ) 
  {
      // #define UPDATE_SIZE_UNKNOWN 0xFFFFFFFF
    return 0xFFFFFFFF;
  }

  virtual void reset( void ) 
  {
    ESP.restart();
  }
  
  virtual void eraseConfig( void ) 
  {
  
  }
    
  virtual String getChipInfo( void )
  {

    esp_chip_info_t info;
    esp_chip_info(&info);

    String strMsg;

    switch( info.model )
    {
      case CHIP_ESP32: 
        {
          strMsg = String("ESP32"); 
        } break;

      case CHIP_ESP32S2: 
        {
          strMsg = String("ESP32S2"); 
        } break;

      case CHIP_ESP32S3: 
        {
          strMsg = String("ESP32S3"); 
        } break;

      case CHIP_ESP32C3: 
        {
          strMsg = String("ESP32C3"); 
        } break;

      case CHIP_ESP32H2: 
        {
          strMsg = String("ESP32H2"); 
        } break;
    }

    String infoString = String(CHIP_ESP32 == info.model ? "ESP32" : strMsg.c_str()) +
                             ", rev: " + String(info.revision) +
                             ", " + String(info.cores) + " core";

    if(info.features & CHIP_FEATURE_WIFI_BGN) 
    {
      infoString += ", WiFi";
    }

    if(info.features & CHIP_FEATURE_BLE) 
    {
      infoString += ", BLE";
    }

    if(info.features & CHIP_FEATURE_BT) 
    {
      infoString += ", BT";
    }

    if(info.features & CHIP_FEATURE_EMB_FLASH) 
    {
      infoString += ", Flash";
    }

    #ifdef BOARD_HAS_PSRAM
      infoString += ", PSRAM: " + String( ESP.getPsramSize () ) +
                    ", FREE PSRAM: " + String( ESP.getFreePsram () );
    #endif // BOARD_HAS_PSRAM

    return infoString;
  }

};  /* end of the class */
