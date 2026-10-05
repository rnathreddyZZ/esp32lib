#include "ZZSettings.h"
#include "LittleFS.h"
#include "ZZDebug.h"

#include <json/AXJSON.h>
#include <string.h>
#include <set>

#define SETTINGS_FILE_PATH "/Settings/ZZSettings.bin"
#define SETTINGS_TMP_PATH  "/Settings/ZZSettings.bin.tmp"

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
    stParams ostParams = it->second;

    for( auto i = ostParams.begin( ); i != ostParams.end( ); ++i )
    {
      stParam ostParam = *i;
      ZZ_DBG_INFO( "%s %s %s\n", it->first.c_str( ),
                           ostParam.name_str.c_str( ),
                              ostParam.value_str.c_str( ));
    }
    ++it;
  }
}


void ZZSettings::setParam( const char *section_pch,
                           const char *name_pch,
                           const char *value_pch )
{

  std::map<string, vector<stParam>>::iterator it = m_section_ost.begin();
  while( it != m_section_ost.end( ) )
  {
    if( 0 == strcmp( section_pch, it->first.c_str( ) ))
    {
      vector<stParam> ostParams = it->second;
      int iInd = 0;
      for( auto i = ostParams.begin( ); i != ostParams.end( ); ++i )
      {
        stParam ostParam = *i;
        if( 0 == strcmp( ostParam.name_str.c_str( ), name_pch ) )
        {
          ostParam.value_str = value_pch;
          ostParams[iInd]  = ostParam;
          m_section_ost[section_pch] = ostParams;
          break;
        }

        iInd++;
      }
    }

    ++it;
  }
}

void ZZSettings::setParam( const char *section_pch,
                           const char *name_pch,
                           int32_t value_i32 )
{

  char value_pch[128];

  sprintf ( value_pch, "%d", value_i32 );
  setParam( section_pch, name_pch, value_pch );
}


bool ZZSettings::getParam( const char* sec_pch,
                           const char *name_pch,
                           stParam& param_ost )
{

  bool bRCode = false;
  stParam* param_pst = NULL;

  std::map<string, vector<stParam>>::iterator it;

  it = m_section_ost.find( sec_pch );
  if( it != m_section_ost.end( ) )
  {
    stParams ostParams = it->second;
    stParamIter iter;
    iter = std::find_if( std::begin(ostParams), std::end(ostParams),
                          [name_pch](const stParam & item)
                            {
                              return ( 0 == strcmp( name_pch, item.name_str.c_str( ) ) );
                            } );
    if( iter != std::end( ostParams ) )
    {
      param_ost = *iter;
      bRCode = true;
    }
  }

  return bRCode;
}


bool ZZSettings::getParam( const char* sec_pch,
                           const char *name_pch,
                           char* value_pch )
{


  bool bRCode = false;
  stParam param_ost;
  if( true == getParam( sec_pch, name_pch, param_ost ) )
  {
    strcpy( value_pch, param_ost.value_str.c_str() );
    bRCode = true;
  }

  return bRCode;
}

bool ZZSettings::getIntParam( const char* sec_pch,
                              const char *name_pch,
                              uint32_t& value_u32 )
{

  bool rcode_b = false;
  stParam param_ost;

  if( true == getParam( sec_pch, name_pch, param_ost ) )
  {
    if( enmDataTypeInt == param_ost.type_u8 )
    {
      if( param_ost.value_str.length( ) > 0 )
      {
        value_u32 = stoi( param_ost.value_str );
        rcode_b = true;
      }
    }
  }

  return rcode_b;
}


bool ZZSettings::getDblParam( const char* sec_pch,
                              const char *name_pch,
                              double& value_d )
{

  bool rcode_b = false;
  stParam param_ost;

  if( true == getParam( sec_pch, name_pch, param_ost ) )
  {
    if( enmDataTypeDouble == param_ost.type_u8 )
    {
      if( param_ost.value_str.length( ) > 0 )
      {
        value_d = stof( param_ost.value_str );
        rcode_b = true;
      }
    }
  }

  return rcode_b;
}


bool ZZSettings::getBoolParam( const char* sec_pch,
                               const char *name_pch,
                               bool& value_b )
{

  bool rcode_b = false;
  stParam param_ost;

  if( true == getParam( sec_pch, name_pch, param_ost ) )
  {
    if( enmDataTypeBool == param_ost.type_u8 )
    {
      if( param_ost.value_str.length() > 0 )
      {
        if( ( param_ost.value_str == "true" ) ||
                     ( param_ost.value_str == "1" ) )
        {
          value_b = true;
          rcode_b = true;
        }
        else if( ( param_ost.value_str == "false" ) ||
                         ( param_ost.value_str == "0" ) )
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

  bool rcode_b = false;
  stParam param_ost;

  if( true == getParam ( sec_pch, name_pch, param_ost ) )
  {
    if( enmDataTypeBool == param_ost.type_u8 )
    {
      if( param_ost.value_str.length( ) > 0 )
      {
        if(( param_ost.value_str == "true") ||
                     ( param_ost.value_str == "1" ) )
        {
          rcode_b = true;
        }
        else if( ( param_ost.value_str == "false" ) ||
                         ( param_ost.value_str == "0" ) )
        {
          rcode_b = true;
        }
      }
    }
  }

  return rcode_b;
}

bool ZZSettings::getRangeByParam( const char* sec_pch,
                                  const char *name_pch,
                                  const char *value_pcch,
                                  uint16_t& value_u16 )
{

  bool rcode_b = false;
  stParam param_ost;

  if( true == getParam( sec_pch, name_pch, param_ost ) )
  {
    if( enmDataTypeInt == param_ost.type_u8 )
    {
      if( 0 == strcmp( value_pcch,"minimum" ) )
      {
        value_u16 = param_ost.min_u16;
        rcode_b = true;
      }
      else if( 0 == strcmp (value_pcch, "maximum") )
      {
        value_u16 = param_ost.max_u16;
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

  stParam param_ost;
  if( true == getParam( sec_pcch, name_pcch, param_ost ) )
  {
    type_enm = param_ost.type_u8;
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
    stParams ostParams = it->second;
    stParamIter iter;
    for( iter = ostParams.begin( ); iter != ostParams.end( ); ++iter )
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
    stParams ostParams = it->second;
    for( auto i = ostParams.begin( ); i != ostParams.end( ); ++i )
    {
      stParam param_ost = *i;

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

void ZZSettings::setData( string data_str )
{

  AXJSON ocJSON;

  json_object *pcObj = nullptr;
  json_object *pcName = nullptr;
  json_object *pcMax = nullptr;
  json_object *pcMin = nullptr;
  json_object *pcType = nullptr;
  json_object *pcReadOnly = nullptr;
  json_object *pcValue = nullptr;

  const char* sec_pcch = nullptr;
  const char* name_pcch = nullptr;
  const char* value_pcch = nullptr;

  ocJSON.init (data_str.c_str( ));

  sec_pcch = ocJSON.getKey( );
  ocJSON.get_json_obj( ocJSON.getKey( ), pcObj );

  uint8_t ucLen = json_object_array_length( pcObj );
  for( uint8_t ucInd = 0; ucInd < ucLen; ucInd++ )
  {

    json_object *pcC = json_object_array_get_idx( pcObj, ucInd );

    json_object_object_get_ex( pcC, "name", &pcName );
    json_object_object_get_ex( pcC, "maximum", &pcMax );
    json_object_object_get_ex( pcC, "minimum", &pcMin );
    json_object_object_get_ex( pcC, "type", &pcType );
    json_object_object_get_ex( pcC, "readonly", &pcReadOnly );
    json_object_object_get_ex( pcC, "value", &pcValue );

    name_pcch = json_object_get_string( pcName );
    value_pcch = json_object_get_string( pcValue );

    setParam( sec_pcch, name_pcch, value_pcch );
  }

  ocJSON.clear( );
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
    auto write = [&]( const void* pvData, size_t uLen )
    {
      if( bOk && file.write( ( const uint8_t* )pvData, uLen ) != uLen )
      {
        bOk = false;
      }
    };

    std::map<string, vector<stParam>>::iterator it = m_section_ost.begin( );

     int32_t iLen1 = m_section_ost.size( );
     write( &iLen1, sizeof( int32_t ) );

    for( ; it != m_section_ost.end( ); ++it )
    {
     int32_t iLen = it->first.length( );
     write( &iLen, sizeof( int32_t ) );
     write( it->first.c_str( ), iLen );

     vector<stParam> ostParams = it->second;

     int32_t iCnt = ostParams.size( );
     write( &iCnt, sizeof( int32_t ) );

     for( auto i = ostParams.begin( ); i != ostParams.end( ); ++i )
     {
      stParam ostParam = *i;

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
    ZZ_DBG_INFO( "Device configuration not found. Using the factory set configuration\n" )
    m_section_ost.clear( );
    init( );
    save( );
    return;
  }

  ZZ_DBG_INFO("Initializing configuration file...\n" );

    /* every read is checked, and every length is validated
       against the bytes left in the file, so a truncated or
       corrupted file can not overrun memory */
  auto readN = [&]( void* pvData, size_t uLen ) -> bool
  {
    return file.read( ( uint8_t* )pvData, uLen ) == uLen;
  };
  auto readStr = [&]( string& str, int32_t iLen ) -> bool
  {
    if( iLen < 0 || iLen > file.available( ) )
    {
      return false;
    }
    str.resize( iLen );
    return iLen == 0 || readN( &str[0], iLen );
  };

  bool bOk = readN( &len_u32, sizeof( int32_t ) ) && len_u32 >= 0;
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

  file.close( );

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
       firmware are kept, so a downgrade/upgrade does not lose them */
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
        iter->value_str = ostLoadedParam.value_str;
        ostMatched.insert( { sec.first, ostLoadedParam.name_str } );
      }
      else
      {
        ostParams.push_back( ostLoadedParam );
      }
    }
  }

    /* persist the defaults of newly added parameters */
  if( ostMatched.size( ) < uDefaults )
  {
    ZZ_DBG_INFO( "Adding %u new parameter(s) to the configuration file\n",
                 (unsigned)( uDefaults - ostMatched.size( ) ) );
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
  append("application", "ws_address", 0, 24, false, enmDataTypeString,"49.207.12.100");
  append("application", "ws_port", 0, 65535, false, enmDataTypeInt, "8080");
  append("application", "ws_URL", 0, 1024, false, enmDataTypeString, "/fieldsync/device/SN123456789");
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
