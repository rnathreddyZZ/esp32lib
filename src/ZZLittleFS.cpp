#include "ZZLittleFS.h"
#include <LittleFS.h>
#include <esp_littlefs.h>
#include <esp_partition.h>
#include <ZZDebug.h>
#include <ZZESPAL.h>

  /* must match the FS partition in the partition table */
#define FS_PARTITION_LABEL "spiffs"
#define FS_BASE_PATH       "/littlefs"
#define FS_MAX_OPEN_FILES  10

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

static bool mountLittleFS( void )
{
  return LittleFS.begin( false, FS_BASE_PATH, FS_MAX_OPEN_FILES,
                         FS_PARTITION_LABEL );
}

  /* repeat the mount outside LittleFS.begin( ), which hides
     the error code; unmounts again if the mount succeeds */
static esp_err_t probeMount( void )
{
  esp_vfs_littlefs_conf_t conf = { };
  conf.base_path = FS_BASE_PATH;
  conf.partition_label = FS_PARTITION_LABEL;
  conf.grow_on_mount = true;

  esp_err_t err = esp_vfs_littlefs_register( &conf );
  if( ESP_OK == err )
  {
    esp_vfs_littlefs_unregister( FS_PARTITION_LABEL );
  }
  return err;
}

bool initLittleFS( )
{

    /* mount without formatting first, so that a format
       (which erases all stored settings) is always logged */
  if( false == mountLittleFS( ) )
  {
    if( NULL == esp_partition_find_first( ESP_PARTITION_TYPE_DATA,
                                          ESP_PARTITION_SUBTYPE_DATA_SPIFFS,
                                          FS_PARTITION_LABEL ) )
    {
      ZZ_DBG_ERR( "LittleFS partition '%s' not found\n", FS_PARTITION_LABEL );
      return false;
    }

      /* only ESP_FAIL means the partition content is unusable;
         any other error (e.g. ESP_ERR_NO_MEM) must not wipe it */
    esp_err_t err = probeMount( );
    if( ESP_OK == err )
    {
      if( false == mountLittleFS( ) )
      {
        ZZ_DBG_ERR( "LittleFS mount failed\n" );
        return false;
      }
    }
    else if( ESP_FAIL != err )
    {
      ZZ_DBG_ERR( "LittleFS mount failed: %s\n", esp_err_to_name( err ) );
      return false;
    }
    else
    {
      ZZ_DBG_ERR( "LittleFS mount failed, formatting\n" );
      if( false == LittleFS.format( ) || false == mountLittleFS( ) )
      {
        ZZ_DBG_ERR( "LittleFS format/mount failed\n" );
        return false;
      }
      ZZ_DBG_WARN( "LittleFS formatted - stored settings lost\n" );
    }
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
