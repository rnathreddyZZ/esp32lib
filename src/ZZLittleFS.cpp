#include "ZZLittleFS.h"
#include <ZZDebug.h>
#include <ZZESPAL.h>

  /* function to create folder with the given name */
void createDir( fs::FS &fs, const char * path )
{

  ZZ_DBG_INFO( "Creating Dir: %s\n", path );
  if( fs.mkdir( path ) )
    { 
      ZZ_DBG_INFO( "Dir created\n" ); 
    }
  else 
    { 
      ZZ_DBG_ERR( "mkdir failed\n" ); 
    }
}

  /* function to save the given data of length to 
        the given filename */
void saveFile( const char* pcchFName, 
               const char* pchData, int32_t /*iLen*/ )
{

  ZZ_DBG_INFO( "Writing file: %s\n", pcchFName );
  File file = LittleFS.open( pcchFName, FILE_WRITE );

  if( !file )
  { 
    ZZ_DBG_ERR( "- failed to open file for appending\n" ); 
  }
  else 
  {
    if( file.print( pchData ) )
      { 
        ZZ_DBG_INFO( "- file written\n" ); 
      }
    else 
      { 
        ZZ_DBG_INFO( "- write failed\n" ); 
      }
  
    file.close( );
  }
}

  /* function to read/retrieve the data from the
        given filename */
void getFile( const char* pcchFName, 
              char** pchData, 
              int32_t *piLen )
{

  ZZ_DBG_DEBUG( "Reading file: %s\n", pcchFName );

  *piLen = 0;
  File file = LittleFS.open( pcchFName );
  if( !file )
  {
    ZZ_DBG_ERR( "- failed to open file for reading\n" );
  }
  else 
  {
    int32_t jsonDataIndx = 0;
    char jsonData[1024*2] = {0,};

    while( file.available( ) )
    { 
      jsonData[jsonDataIndx++] = file.read( );
    }

    if( jsonDataIndx != 0 )
    {
      *pchData = jsonData; 
      *piLen = jsonDataIndx;
    }
    file.close( );
    ZZ_DBG_DEBUG( "Reading file done: %s\n", pcchFName );
  }
}

void initLittleFS( )
{

  if( false == LittleFS.begin( true ) )
  { 
    ZZ_DBG_ERR( "LittleFS Mount Failed\n" );
  }
  else
  { 
    ZZ_DBG_DEBUG( "LittleFS file system mounted\n" ); 
  }

  ZZ_DBG_INFO( "Total Bytes: %d\n", LittleFS.totalBytes( ) );
  ZZ_DBG_INFO( "Used Bytes: %d\n", LittleFS.usedBytes( ) );

  #ifdef BOARD_HAS_PSRAM
    ZZ_DBG_INFO ("Total PSRAM: %d\n", ESP.getPsramSize( ));
    ZZ_DBG_INFO ("Free PSRAM: %d\n",  ESP.getFreePsram( ));
  #endif // BOARD_HAS_PSRAM

  if( false == LittleFS.exists( "/Settings" ) ) 
  { 
    createDir( LittleFS, "/Settings" );
  }
  else
  { 
    ZZ_DBG_DEBUG( "Settings folder already exist.\n" ); 
  }
}
