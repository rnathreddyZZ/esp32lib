#include "ZZSettings.h"
#include "LittleFS.h"
#include "ZZDebug.h"

#include <json/AXJSON.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <algorithm>
#include <set>

using std::string;
using std::vector;
using std::pair;

#define SETTINGS_FILE_PATH "/Settings/ZZSettings.bin"
#define SETTINGS_TMP_PATH  "/Settings/ZZSettings.bin.tmp"

  /* the file ends with a trailer that marks the format and holds
     a CRC-32 of everything before it. it goes at the end, not the
     start, because firmware released before it stops reading after
     the last parameter; a downgrade can still read the file */
#define SETTINGS_MAGIC_U32   0x5A5A53E7u
#define SETTINGS_VERSION_U16 1
#define SETTINGS_MAX_SIZE    ( 32 * 1024 )

typedef struct __attribute__( ( packed ) ) sttrailer
{
  uint32_t magic_u32;
  uint16_t version_u16;
  uint16_t reserved_u16;
  uint32_t crc_u32;
}stTrailer;

ZZSettings::ZZSettings( )
{

}

ZZSettings::~ZZSettings( )
{

}

void ZZSettings::append( const char *section_pch,
                         const char *name_pch,
                         const uint16_t min_u16,
                         const uint16_t max_u16,
                         const bool readonly_b,
                         const uint8_t type_u8,
                         const char *value_pch )
{

  if( NULL == section_pch )
  {
    ZZ_DBG_ERR( "append: no section given\n" );
    return;
  }

    /* value-initialise; memset would corrupt the string members */
  stParam ostParam{ };

  ostParam.min_u16 = min_u16;
  ostParam.max_u16 = max_u16;
  ostParam.readonly_b = readonly_b;
  ostParam.type_u8 = ( enmDataType )type_u8;

  if( NULL != value_pch )
  {
    ostParam.value_str = value_pch;
  }

  if( NULL != name_pch )
  {
    ostParam.name_str = name_pch;
  }

  m_section_ost[section_pch].push_back( ostParam );
}


void ZZSettings::dump( void )
{

  std::map<string, vector<stParam>>::iterator it = m_section_ost.begin( );
  while( it != m_section_ost.end( ) )
  {
    const stParams& ostParams = it->second;

    for( auto i = ostParams.begin( ); i != ostParams.end( ); ++i )
    {
      const stParam& ostParam = *i;
      ZZ_DBG_INFO( "%s %s %s\n", it->first.c_str( ),
                           ostParam.name_str.c_str( ),
                              ostParam.value_str.c_str( ));
    }
    ++it;
  }
}


stParam* ZZSettings::findParam( const char* sec_pch,
                                const char* name_pch )
{

  if( NULL == sec_pch || NULL == name_pch )
  {
    return NULL;
  }

  std::map<string, vector<stParam>>::iterator it = m_section_ost.find( sec_pch );
  if( it == m_section_ost.end( ) )
  {
    return NULL;
  }

  stParamIter iter = std::find_if( it->second.begin( ), it->second.end( ),
                                   [name_pch]( const stParam& item )
                                   {
                                     return item.name_str == name_pch;
                                   } );

  return ( iter != it->second.end( ) ) ? &( *iter ) : NULL;
}


  /* int: whole decimal number within [min, max]
     double: whole decimal number
     bool: true/false/1/0
     string: length within [min, max] */
bool ZZSettings::isValidValue( const stParam& param_ost,
                               const char* value_pch )
{

  if( NULL == value_pch )
  {
    return false;
  }

  char* end_pch = NULL;

  switch( param_ost.type_u8 )
  {
    case enmDataTypeBool:
      return ( 0 == strcmp( value_pch, "true" ) ) ||
             ( 0 == strcmp( value_pch, "false" ) ) ||
             ( 0 == strcmp( value_pch, "1" ) ) ||
             ( 0 == strcmp( value_pch, "0" ) );

    case enmDataTypeInt:
    {
      errno = 0;
      long value_l = strtol( value_pch, &end_pch, 10 );
      return ( end_pch != value_pch ) && ( '\0' == *end_pch ) &&
             ( 0 == errno ) &&
             ( value_l >= param_ost.min_u16 ) &&
             ( value_l <= param_ost.max_u16 );
    }

    case enmDataTypeDouble:
      errno = 0;
      strtod( value_pch, &end_pch );
      return ( end_pch != value_pch ) && ( '\0' == *end_pch ) &&
             ( 0 == errno );

    case enmDataTypeString:
    {
      size_t len_u = strlen( value_pch );
      return ( len_u >= param_ost.min_u16 ) && ( len_u <= param_ost.max_u16 );
    }

    default:
      return false;
  }
}


bool ZZSettings::setParam( const char *section_pch,
                           const char *name_pch,
                           const char *value_pch )
{

  stParam* param_pst = findParam( section_pch, name_pch );
  if( NULL == param_pst )
  {
    ZZ_DBG_ERR( "setParam: unknown parameter\n" );
    return false;
  }

    /* never log the value; it may be a password */
  if( false == isValidValue( *param_pst, value_pch ) )
  {
    ZZ_DBG_ERR( "setParam: invalid value for %s/%s\n", section_pch, name_pch );
    return false;
  }

  param_pst->value_str = value_pch;
  return true;
}

bool ZZSettings::setParam( const char *section_pch,
                           const char *name_pch,
                           int32_t value_i32 )
{

  char value_pch[128];

  sprintf ( value_pch, "%d", value_i32 );
  return setParam( section_pch, name_pch, value_pch );
}


bool ZZSettings::getParam( const char* sec_pch,
                           const char *name_pch,
                           char* value_pch )
{

  stParam* param_pst = findParam( sec_pch, name_pch );
  if( NULL == param_pst )
  {
    return false;
  }

  strcpy( value_pch, param_pst->value_str.c_str() );
  return true;
}

bool ZZSettings::getIntParam( const char* sec_pch,
                              const char *name_pch,
                              uint32_t& value_u32 )
{

    /* the stored value may come from an old or corrupted file;
       isValidValue rejects anything that is not a whole number
       within [min, max], so the conversion can not fail or wrap */
  stParam* param_pst = findParam( sec_pch, name_pch );

  if( ( NULL == param_pst ) ||
      ( enmDataTypeInt != param_pst->type_u8 ) ||
      ( false == isValidValue( *param_pst, param_pst->value_str.c_str( ) ) ) )
  {
    return false;
  }

  value_u32 = strtoul( param_pst->value_str.c_str( ), NULL, 10 );
  return true;
}


bool ZZSettings::getDblParam( const char* sec_pch,
                              const char *name_pch,
                              double& value_d )
{

  stParam* param_pst = findParam( sec_pch, name_pch );

  if( ( NULL == param_pst ) ||
      ( enmDataTypeDouble != param_pst->type_u8 ) ||
      ( false == isValidValue( *param_pst, param_pst->value_str.c_str( ) ) ) )
  {
    return false;
  }

    /* strtod, not stof: keeps double precision and never throws */
  value_d = strtod( param_pst->value_str.c_str( ), NULL );
  return true;
}


bool ZZSettings::getBoolParam( const char* sec_pch,
                               const char *name_pch,
                               bool& value_b )
{

  bool rcode_b = false;
  stParam* param_pst = findParam( sec_pch, name_pch );

  if( NULL != param_pst )
  {
    if( enmDataTypeBool == param_pst->type_u8 )
    {
      if( param_pst->value_str.length() > 0 )
      {
        if( ( param_pst->value_str == "true" ) ||
                     ( param_pst->value_str == "1" ) )
        {
          value_b = true;
          rcode_b = true;
        }
        else if( ( param_pst->value_str == "false" ) ||
                         ( param_pst->value_str == "0" ) )
        {
          value_b = false;
          rcode_b = true;
        }
      }
    }
  }

  return rcode_b;
}

bool ZZSettings::isReadOnly( const char* sec_pch,
                             const char* name_pch )
{

  stParam* param_pst = findParam( sec_pch, name_pch );

  return ( NULL != param_pst ) && param_pst->readonly_b;
}

bool ZZSettings::getRangeByParam( const char* sec_pch,
                                  const char *name_pch,
                                  const char *value_pcch,
                                  uint16_t& value_u16 )
{

  bool rcode_b = false;
  stParam* param_pst = findParam( sec_pch, name_pch );

  if( NULL != param_pst )
  {
    if( enmDataTypeInt == param_pst->type_u8 )
    {
      if( 0 == strcmp( value_pcch,"minimum" ) )
      {
        value_u16 = param_pst->min_u16;
        rcode_b = true;
      }
      else if( 0 == strcmp (value_pcch, "maximum") )
      {
        value_u16 = param_pst->max_u16;
        rcode_b = true;
      }
    }
  }
  return rcode_b;
}

enmDataType ZZSettings::getType( const char* sec_pcch,
                                 const char* name_pcch )
{

  enmDataType type_enm = enmDataTypeNone;

  stParam* param_pst = findParam( sec_pcch, name_pcch );
  if( NULL != param_pst )
  {
    type_enm = param_pst->type_u8;
  }

  return type_enm;
}

std::vector<string> ZZSettings::getSections( )
{

  vector<string> sections;

  std::map<string, vector<stParam>>::iterator it = m_section_ost.begin();
  while ( it != m_section_ost.end( ) )
  {
    sections.push_back( it->first );
    ++it;
  }

  return sections;
}

std::vector<string> ZZSettings::getKeys( const char* sec_pcch )
{

  std::vector<string> strKeys;
  std::map<string, vector<stParam>>::iterator it;

  it = m_section_ost.find ( sec_pcch );
  if( it != m_section_ost.end( ) )
  {
    const stParams& ostParams = it->second;
    for( auto iter = ostParams.begin( ); iter != ostParams.end( ); ++iter )
    {
      strKeys.push_back( iter->name_str );
    }
  }

  return strKeys;
}

string ZZSettings::getData( string sec_str )
{

  string data_str;

  AXJSON ocJSON,
         ocJSONArray( true );

  bool bReadOnly = false;

  ocJSON.newObj( );
  ocJSONArray.newObj( );

  std::map<string, vector<stParam>>::iterator it;

  it = m_section_ost.find( sec_str );
  if( it != m_section_ost.end( ) )
  {
    const stParams& ostParams = it->second;
    for( auto i = ostParams.begin( ); i != ostParams.end( ); ++i )
    {
      const stParam& param_ost = *i;

      AXJSON ocObjKeyValue;
      ocObjKeyValue.newObj( );

      ocObjKeyValue.add_string( "name", param_ost.name_str.c_str( ) );
      ocObjKeyValue.add_string( "value", param_ost.value_str.c_str( ) );
      ocObjKeyValue.add_int( "maximum", param_ost.max_u16 );
      ocObjKeyValue.add_int( "minimum", param_ost.min_u16 );
      ocObjKeyValue.add_int( "type", param_ost.type_u8 );
      ocObjKeyValue.add_int( "readonly", param_ost.readonly_b );
      ocJSONArray.add( NULL, ocObjKeyValue.get_json_obj () );
    }

    ocJSON.add( sec_str.c_str( ), ocJSONArray.get_json_obj( ) );
    data_str = ocJSON.get( );
  }

  ocJSON.clear( );
  return data_str;
}

  /* expects the getData( ) format, one section per request:
     { "<section>": [ { "name": "...", "value": "..." }, ... ] }
     the other fields (range, type, read-only) are ignored; the
     firmware defines them */
bool ZZSettings::setData( string data_str )
{

  AXJSON ocJSON;

  if( false == ocJSON.init( data_str.c_str( ) ) )
  {
    ZZ_DBG_ERR( "setData: invalid JSON\n" );
    return false;
  }

  json_object *pcRoot = ocJSON.get_json_obj( );
  json_object *pcArray = nullptr;
  const char* sec_pcch = nullptr;

  if( json_object_is_type( pcRoot, json_type_object ) &&
      ( 1 == json_object_object_length( pcRoot ) ) )
  {
    json_object_object_foreach( pcRoot, key_pcch, val_pcObj )
    {
      sec_pcch = key_pcch;
      pcArray = val_pcObj;
    }
  }

  if( !json_object_is_type( pcArray, json_type_array ) )
  {
    ZZ_DBG_ERR( "setData: expected one section with an array\n" );
    ocJSON.clear( );
    return false;
  }

    /* validate every entry before changing anything, so a
       bad request never leaves the settings half updated */
  vector<pair<stParam*, string>> ostUpdates;
  bool rcode_b = true;
  size_t len_u = json_object_array_length( pcArray );

  for( size_t ind_u = 0; rcode_b && ind_u < len_u; ind_u++ )
  {
    json_object *pcEntry = json_object_array_get_idx( pcArray, ind_u );
    json_object *pcName = nullptr;
    json_object *pcValue = nullptr;

    const char* name_pcch = nullptr;
    const char* value_pcch = nullptr;

    if( json_object_is_type( pcEntry, json_type_object ) &&
        json_object_object_get_ex( pcEntry, "name", &pcName ) &&
        json_object_object_get_ex( pcEntry, "value", &pcValue ) )
    {
      name_pcch = json_object_get_string( pcName );
      value_pcch = json_object_get_string( pcValue );
    }

    stParam* param_pst = findParam( sec_pcch, name_pcch );

    if( NULL == param_pst )
    {
      ZZ_DBG_ERR( "setData: entry %u is not a known parameter\n", (unsigned)ind_u );
      rcode_b = false;
    }
    else if( param_pst->readonly_b )
    {
      ZZ_DBG_ERR( "setData: %s/%s is read-only\n", sec_pcch, name_pcch );
      rcode_b = false;
    }
    else if( false == isValidValue( *param_pst, value_pcch ) )
    {
      ZZ_DBG_ERR( "setData: invalid value for %s/%s\n", sec_pcch, name_pcch );
      rcode_b = false;
    }
    else
    {
      ostUpdates.push_back( { param_pst, value_pcch } );
    }
  }

  if( rcode_b )
  {
    for( auto& update : ostUpdates )
    {
      update.first->value_str = update.second;
    }
  }

  ocJSON.clear( );
  return rcode_b;
}

  /* CRC-32 (IEEE), chainable: crc32( crc32( 0, a ), b ) == crc32( 0, ab ) */
static uint32_t crc32( uint32_t crc_u32, const uint8_t* data_puc, size_t len_u )
{

  crc_u32 = ~crc_u32;
  while( len_u-- )
  {
    crc_u32 ^= *data_puc++;
    for( int bit_i = 0; bit_i < 8; bit_i++ )
    {
      crc_u32 = ( crc_u32 >> 1 ) ^ ( 0xEDB88320u & ( 0u - ( crc_u32 & 1u ) ) );
    }
  }

  return ~crc_u32;
}

void ZZSettings::save( )
{

    /* write to a temporary file and rename it over the
       settings file, so a power loss never leaves a
       truncated settings file behind */
  File file = LittleFS.open( SETTINGS_TMP_PATH, FILE_WRITE );
  if (!file)
  {
    ZZ_DBG_ERR( "Failed to open the file\n" );
  }
  else
  {
    bool bOk = true;
    uint32_t crc_u32 = 0;
    auto write = [&]( const void* pvData, size_t uLen )
    {
      if( bOk && file.write( ( const uint8_t* )pvData, uLen ) != uLen )
      {
        bOk = false;
      }
      crc_u32 = crc32( crc_u32, ( const uint8_t* )pvData, uLen );
    };

    std::map<string, vector<stParam>>::iterator it = m_section_ost.begin( );

     int32_t iLen1 = m_section_ost.size( );
     write( &iLen1, sizeof( int32_t ) );

    for( ; it != m_section_ost.end( ); ++it )
    {
     int32_t iLen = it->first.length( );
     write( &iLen, sizeof( int32_t ) );
     write( it->first.c_str( ), iLen );

     const stParams& ostParams = it->second;

     int32_t iCnt = ostParams.size( );
     write( &iCnt, sizeof( int32_t ) );

     for( auto i = ostParams.begin( ); i != ostParams.end( ); ++i )
     {
      const stParam& ostParam = *i;

      write( &ostParam.max_u16, sizeof( int16_t ) );
      write( &ostParam.min_u16, sizeof( int16_t ) );
      write( &ostParam.type_u8, sizeof( int8_t ) );
      write( &ostParam.readonly_b, sizeof( int8_t ) );

      int16_t ln_u16 = ostParam.name_str.length( );
      write( &ln_u16, sizeof( int16_t ) );
      write( ostParam.name_str.c_str( ), ln_u16 );

      ln_u16 = ostParam.value_str.length( );
      write( &ln_u16, sizeof( int16_t ) );
      write( ostParam.value_str.c_str( ), ln_u16 );
     }
    }

    stTrailer trailer_ost = { SETTINGS_MAGIC_U32, SETTINGS_VERSION_U16, 0, crc_u32 };
    write( &trailer_ost, sizeof( stTrailer ) );

    file.close();

      /* LittleFS rename replaces the target atomically */
    if( bOk && LittleFS.rename( SETTINGS_TMP_PATH, SETTINGS_FILE_PATH ) )
    {
      ZZ_DBG_INFO( "Settings saved\n" );
    }
    else
    {
      ZZ_DBG_ERR( "Failed to save the settings\n" );
      LittleFS.remove( SETTINGS_TMP_PATH );
    }
  }
}

void ZZSettings::load( )
{

  int32_t len_u32 = 0;
  int32_t secs_u32 = 0;
  int32_t params_u32 = 0;
  int16_t len_u16 = 0;
  uint16_t max_u16 = 0;
  uint16_t min_u16 = 0;
  uint8_t type_u8 = 0;
  uint8_t readonly_b = 0;

  string strSec;
  string strName;
  string strVal;

  std::map<string, stParams> ostLoaded;

  File file = LittleFS.open( SETTINGS_FILE_PATH, FILE_READ );

  if( !file )
  {
    ZZ_DBG_INFO( "Device configuration not found. Using the factory set configuration\n" );
    m_section_ost.clear( );
    init( );
    save( );
    return;
  }

  ZZ_DBG_INFO("Initializing configuration file...\n" );

    /* read the whole file, so the trailer can be checked
       before anything is parsed */
  size_t size_u = file.size( );
  vector<uint8_t> data_vuc;
  bool bOk = ( size_u <= SETTINGS_MAX_SIZE );
  if( bOk )
  {
    data_vuc.resize( size_u );
    bOk = ( 0 == size_u ) || ( file.read( data_vuc.data( ), size_u ) == size_u );
  }

  file.close( );

    /* no trailer: written by firmware released before the
       trailer was added. parse it as is and rewrite it */
  size_t payload_u = size_u;
  bool bLegacy = true;
  if( bOk && size_u >= sizeof( stTrailer ) )
  {
    stTrailer trailer_ost;
    memcpy( &trailer_ost, &data_vuc[size_u - sizeof( stTrailer )], sizeof( stTrailer ) );
    if( SETTINGS_MAGIC_U32 == trailer_ost.magic_u32 )
    {
      bLegacy = false;
      payload_u = size_u - sizeof( stTrailer );
      bOk = ( SETTINGS_VERSION_U16 == trailer_ost.version_u16 ) &&
            ( crc32( 0, data_vuc.data( ), payload_u ) == trailer_ost.crc_u32 );
    }
  }

    /* every read is checked, and every length is validated
       against the bytes left in the payload, so a truncated or
       corrupted file can not overrun memory */
  size_t pos_u = 0;
  auto readN = [&]( void* pvData, size_t uLen ) -> bool
  {
    if( uLen > payload_u - pos_u )
    {
      return false;
    }
    memcpy( pvData, data_vuc.data( ) + pos_u, uLen );
    pos_u += uLen;
    return true;
  };
  auto readStr = [&]( string& str, int32_t iLen ) -> bool
  {
    if( iLen < 0 || ( size_t )iLen > payload_u - pos_u )
    {
      return false;
    }
    str.assign( ( const char* )data_vuc.data( ) + pos_u, iLen );
    pos_u += iLen;
    return true;
  };

  bOk = bOk && readN( &len_u32, sizeof( int32_t ) ) && len_u32 >= 0;
  secs_u32 = len_u32;

  for( int32_t sec_u32 = 0; bOk && sec_u32 < secs_u32; sec_u32++ )
  {
   bOk = readN( &len_u32, sizeof( int32_t ) ) &&
         readStr( strSec, len_u32 ) &&
         readN( &params_u32, sizeof( int32_t ) ) &&
         params_u32 >= 0;

   for( int32_t param_u32 = 0; bOk && param_u32 < params_u32; param_u32++ )
   {
    bOk = readN( &max_u16, sizeof( int16_t ) ) &&
          readN( &min_u16, sizeof( int16_t ) ) &&
          readN( &type_u8, sizeof( int8_t ) ) &&
          readN( &readonly_b, sizeof( int8_t ) ) &&
          readN( &len_u16, sizeof( int16_t ) ) &&
          readStr( strName, len_u16 ) &&
          readN( &len_u16, sizeof( int16_t ) ) &&
          readStr( strVal, len_u16 );

    if( bOk )
    {
      stParam ostParam{ };
      ostParam.name_str   = strName;
      ostParam.min_u16    = min_u16;
      ostParam.max_u16    = max_u16;
      ostParam.readonly_b = readonly_b;
      ostParam.type_u8    = ( enmDataType )type_u8;
      ostParam.value_str  = strVal;
      ostLoaded[strSec].push_back( ostParam );
    }
   }
  }

  if( false == bLegacy )
  {
    bOk = bOk && ( pos_u == payload_u );
  }

    /* start from the factory set configuration, so that
       parameters added by a newer firmware are always present */
  m_section_ost.clear( );
  init( );

  if( false == bOk )
  {
    ZZ_DBG_ERR( "Configuration file corrupted. Restoring the factory set configuration\n" );
    save( );
    return;
  }

    /* overlay the stored values. the firmware defines the
       parameter attributes (range, type, read-only), the file
       only supplies the value. parameters not known to this
       firmware are kept, so a downgrade/upgrade does not lose them.
       a stored value that is not valid is replaced by the
       factory value (known parameters) or dropped (unknown ones) */
  bool bSave = bLegacy;
  std::set<std::pair<string, string>> ostMatched;
  size_t uDefaults = 0;
  for( auto& sec : m_section_ost )
  {
    uDefaults += sec.second.size( );
  }

  for( auto& sec : ostLoaded )
  {
    for( auto& ostLoadedParam : sec.second )
    {
      stParams& ostParams = m_section_ost[sec.first];
      auto iter = std::find_if( ostParams.begin( ), ostParams.end( ),
                                [&]( const stParam& item )
                                {
                                  return item.name_str == ostLoadedParam.name_str;
                                } );
      if( iter != ostParams.end( ) )
      {
        if( isValidValue( *iter, ostLoadedParam.value_str.c_str( ) ) )
        {
          iter->value_str = ostLoadedParam.value_str;
          ostMatched.insert( { sec.first, ostLoadedParam.name_str } );
        }
        else
        {
          ZZ_DBG_WARN( "Stored %s/%s is invalid. Using the factory value\n",
                       sec.first.c_str( ), ostLoadedParam.name_str.c_str( ) );
        }
      }
      else if( isValidValue( ostLoadedParam, ostLoadedParam.value_str.c_str( ) ) )
      {
        ostParams.push_back( ostLoadedParam );
      }
      else
      {
        ZZ_DBG_WARN( "Stored %s/%s is invalid. Dropping it\n",
                     sec.first.c_str( ), ostLoadedParam.name_str.c_str( ) );
        bSave = true;
      }
    }
  }

    /* persist the factory values of new or invalid parameters */
  if( ostMatched.size( ) < uDefaults )
  {
    ZZ_DBG_INFO( "Saving the factory value of %u parameter(s)\n",
                 (unsigned)( uDefaults - ostMatched.size( ) ) );
    bSave = true;
  }

  if( bLegacy )
  {
    ZZ_DBG_INFO( "Upgrading the configuration file format\n" );
  }

  if( bSave )
  {
    save( );
  }

  ZZ_DBG_INFO( "Initialization done.\n" );
}


void ZZSettings::init( void )
{

  append( "wifi", "ssid", 0, 64, false, enmDataTypeString, "SaNaSW" );
  append( "wifi", "ssid_pwd", 0, 64, false, enmDataTypeString, "SaNa@2022#123456" );
  append( "wifi", "soft_ap_name", 0, 64, false, enmDataTypeString, "zoraiz" );
  append( "wifi", "soft_ap_pwd", 0, 64, false, enmDataTypeString, "zoraiz@123" );
  append( "application", "log_srv_address", 0, 32, false, enmDataTypeString, "192.168.0.31" );
  append( "application", "log_srv_port", 1, 65535, false, enmDataTypeInt, "6600" );
  append("application", "ws_address", 0, 64, false, enmDataTypeString,"49.207.12.100");
  append("application", "ws_port", 0, 65535, false, enmDataTypeInt, "8080");
  append("application", "ws_URL", 0, 1024, false, enmDataTypeString, "/fieldsync/device/SN123456789");
  append("device", "relay_state", 0, 1, false, enmDataTypeBool, "false");
}

  /* global settings file */
ZZSettings *gpcSettings = nullptr;

void initSettings( )
{
    /* other tasks may already hold the pointer; never replace it */
  if( nullptr != gpcSettings )
  {
    ZZ_DBG_WARN( "Settings already initialized\n" );
    return;
  }

  gpcSettings = new ZZSettings;
  gpcSettings->load( );
}
