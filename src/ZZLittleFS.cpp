#include "ZZLittleFS.h"
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

  /* function to save the given data of length to
        the given filename. the data is written to a
        temporary file which is then renamed over the
        target, so a power loss never leaves a truncated file */
bool saveFile( const char* pcchFName,
               const char* pchData, size_t uLen )
{

  ZZ_DBG_INFO( "Writing file: %s\n", pcchFName );

  String strTmp = String( pcchFName ) + ".tmp";
  File file = LittleFS.open( strTmp.c_str( ), FILE_WRITE );

  if( !file )
  {
    ZZ_DBG_ERR( "- failed to open file for writing\n" );
    return false;
  }

  bool bOk = ( file.write( (const uint8_t*)pchData, uLen ) == uLen );
  file.close( );

    /* LittleFS rename replaces the target atomically */
  if( bOk && false == LittleFS.rename( strTmp.c_str( ), pcchFName ) )
  {
    ZZ_DBG_ERR( "- rename failed\n" );
    bOk = false;
  }

  if( bOk )
    {
      ZZ_DBG_INFO( "- file written\n" );
    }
  else
    {
      ZZ_DBG_ERR( "- write failed\n" );
      LittleFS.remove( strTmp.c_str( ) );
    }
  return bOk;
}

  /* function to read/retrieve the data from the
        given filename */
bool getFile( const char* pcchFName,
              char* pchBuf,
              size_t uBufSize,
              size_t *puLen )
{

  ZZ_DBG_DEBUG( "Reading file: %s\n", pcchFName );

  *puLen = 0;
  if( pchBuf == nullptr || uBufSize == 0 )
  {
    return false;
  }
  pchBuf[0] = '\0';

  File file = LittleFS.open( pcchFName );
  if( !file )
  {
    ZZ_DBG_ERR( "- failed to open file for reading\n" );
    return false;
  }

    /* leave room for the NUL terminator */
  size_t uFileSize = file.size( );
  if( uFileSize >= uBufSize )
  {
    ZZ_DBG_ERR( "- file too large: %u bytes, buffer %u\n",
                (unsigned)uFileSize, (unsigned)uBufSize );
    file.close( );
    return false;
  }

  size_t uRead = file.read( (uint8_t*)pchBuf, uFileSize );
  file.close( );
  pchBuf[uRead] = '\0';
  *puLen = uRead;

  ZZ_DBG_DEBUG( "Reading file done: %s\n", pcchFName );
  return uRead == uFileSize;
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
