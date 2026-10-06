#include "ZZLittleFS.h"
#include <LittleFS.h>
#include <ZZDebug.h>
#include <ZZESPAL.h>

  /* function to create folder with the given name */
static bool createDir( fs::FS &fs, const char * path )
{

  ZZ_DBG_INFO( "Creating Dir: %s\n", path );
  if( fs.mkdir( path ) )
    {
      ZZ_DBG_INFO( "Dir created\n" );
      return true;
    }

  ZZ_DBG_ERR( "mkdir failed: %s\n", path );
  return false;
}

bool initLittleFS( )
{

    /* mount without formatting first, so that a format
       (which erases all stored settings) is always logged */
  if( false == LittleFS.begin( false ) )
  {
    ZZ_DBG_ERR( "LittleFS mount failed, formatting\n" );
    if( false == LittleFS.format( ) || false == LittleFS.begin( false ) )
    {
      ZZ_DBG_ERR( "LittleFS format/mount failed\n" );
      return false;
    }
    ZZ_DBG_WARN( "LittleFS formatted - stored settings lost\n" );
  }
  else
  {
    ZZ_DBG_DEBUG( "LittleFS file system mounted\n" );
  }

  ZZ_DBG_INFO( "Total Bytes: %u\n", (unsigned)LittleFS.totalBytes( ) );
  ZZ_DBG_INFO( "Used Bytes: %u\n", (unsigned)LittleFS.usedBytes( ) );

  #ifdef BOARD_HAS_PSRAM
    ZZ_DBG_INFO ("Total PSRAM: %u\n", (unsigned)ESP.getPsramSize( ));
    ZZ_DBG_INFO ("Free PSRAM: %u\n",  (unsigned)ESP.getFreePsram( ));
  #endif // BOARD_HAS_PSRAM

  if( false == LittleFS.exists( "/Settings" ) )
  {
    return createDir( LittleFS, "/Settings" );
  }

  ZZ_DBG_DEBUG( "Settings folder already exist.\n" );
  return true;
}
