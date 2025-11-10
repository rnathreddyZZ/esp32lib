#include "ZZSettings.h"
#include "LittleFS.h"
#include "ZZDebug.h"

#include <json/AXJSON.h>
#include <string.h>

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

  stParam ostParam;

  memset( &ostParam, 0x00, sizeof( ostParam ) );

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
 
  if( NULL != section_pch )
  {
    stParams ostParams = m_section_ost[section_pch];
    ostParams.push_back( ostParam );
    m_section_ost[section_pch] = ostParams;
  }
  else
  {
    stParams ostParams;
    ostParams.push_back( ostParam );
    m_section_ost[section_pch] = ostParams;
  }
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

  File file = LittleFS.open( "/Settings/ZZSettings.bin", FILE_WRITE );
  if (!file)
  {
    ZZ_DBG_INFO( "Failed to open the file\n" );
  }
  else
  {
    std::map<string, vector<stParam>>::iterator it = m_section_ost.begin( );

     int32_t iLen1 = m_section_ost.size( );
     file.write( ( const uint8_t* )&iLen1, sizeof( int32_t ) );
     
    for( ; it != m_section_ost.end( ); ++it )
    {
     int32_t iLen = it->first.length( );
     file.write( ( const uint8_t* )&iLen, sizeof( int32_t ) );
     file.write( ( const uint8_t* )it->first.c_str( ), iLen );

     vector<stParam> ostParams = it->second;
     
     int32_t iCnt = ostParams.size( );
     file.write( ( const uint8_t*)&iCnt, sizeof( int32_t ) );
   
     for( auto i = ostParams.begin( ); i != ostParams.end( ); ++i )
     {
      stParam ostParam = *i;

      file.write( ( const uint8_t* )&ostParam.max_u16, sizeof( int16_t ) );
      file.write( ( const uint8_t* )&ostParam.min_u16, sizeof( int16_t ) );
      file.write( ( const uint8_t* )&ostParam.type_u8, sizeof( int8_t ) );
      file.write( ( const uint8_t* )&ostParam.readonly_b, sizeof( int8_t ) );

      int16_t ln_u16 = ostParam.name_str.length( );
      file.write ( ( const uint8_t* )&ln_u16, sizeof( int16_t ) );
      file.write ( ( const uint8_t* )ostParam.name_str.c_str( ), ln_u16 );

      ln_u16 = ostParam.value_str.length( );
      file.write( ( const uint8_t*)&ln_u16, sizeof(int16_t ) );
      file.write( ( const uint8_t*)ostParam.value_str.c_str( ), ln_u16 );
     }
    }
   
    file.close();
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
 
  char szSec[128];
  char szName[128];
  char szVal[256];

  File file = LittleFS.open( "/Settings/AXSettings.bin", FILE_READ );

  if( !file )
  {
    ZZ_DBG_INFO( "Device configuration not found. Using the factory set configuration\n" )
    init( );
    save( );
  }
  else
  {
    ZZ_DBG_INFO("Initializing configuration file...\n" );
    file.read( ( uint8_t* )&len_u32, sizeof( uint32_t ) );

    secs_u32 = len_u32;
    for( uint32_t sec_u32 = 0; sec_u32 < secs_u32; sec_u32++ )
    {
     file.read( ( uint8_t* )&len_u32, sizeof ( int32_t ) );
     file.read( ( uint8_t* )szSec, len_u32 );
     szSec[len_u32] = '\0';

     file.read( ( uint8_t* )&len_u32, sizeof( int32_t ) );
     params_u32 = len_u32;
     
     for( uint32_t param_u32 = 0; param_u32 < params_u32; param_u32++ )
     {
      file.read( ( uint8_t*)&max_u16, sizeof( int16_t ) );
      file.read( ( uint8_t*)&min_u16, sizeof( int16_t ) );
      file.read( ( uint8_t*)&type_u8, sizeof( int8_t ) );
      file.read( ( uint8_t*)&readonly_b, sizeof( int8_t ) );

      file.read( ( uint8_t* )&len_u16, sizeof( int16_t ) );
      file.read( ( uint8_t* )szName, len_u16 );
      szName[len_u16] = '\0';

      file.read ((uint8_t*)&len_u16, sizeof(int16_t));
      file.read ((uint8_t*)szVal, len_u16);
      szVal[len_u16] = '\0';

      append (szSec, szName, min_u16, max_u16,
                     readonly_b, type_u8, szVal);
     }
    }

    ZZ_DBG_INFO( "Initialization done.\n" );
    file.close( );
  }
}


void ZZSettings::init( void )
{
  append( "wifi", "ssid", 0, 64, false, enmDataTypeString, "Mani" );
  append( "wifi", "ssid_pwd", 0, 64, false, enmDataTypeString, "Mani0404" );
  append( "wifi", "soft_ap_name", 0, 64, false, enmDataTypeString, "MAni" );
  append( "wifi", "soft_ap_pwd", 0, 64, false, enmDataTypeString, "Mani0404" );
  append( "application", "log_srv_address", 0, 32, false, enmDataTypeString, "192.168.0.31" );
  append( "application", "log_srv_port", 1, 65535, false, enmDataTypeInt, "6600" );
  append( "application", "VertualTagId", 0, 21, false, enmDataTypeString, "0000003960856012" );

  append( "ocpp_params", "NumberOfConnectors", 0, 5, false, enmDataTypeInt, "1" );
  append( "ocpp_device", "address", 0, 24, false, enmDataTypeString, "49.207.12.100" ); //20.219.37.15
  append( "ocpp_device", "port", 0, 65535, false, enmDataTypeInt, "8080" ); //9033
  append( "ocpp_device", "path", 0, 1024, false, enmDataTypeString, "/ocpp/16/AX035" ); ///ocpp/AX016
  append( "ocpp_device", "chargeBoxSerialNumber", 0, 32, true, enmDataTypeString, "AXONIFY_000001" );
  append( "ocpp_device", "chargePointModel", 0, 24, true, enmDataTypeString, "211-2G07" );
  append( "ocpp_device", "chargePointSerialNumber", 0, 64, true, enmDataTypeString, "Axonify" );
  append( "ocpp_device", "chargePointVendor", 0, 24, true, enmDataTypeString, "Axonify" );
  append( "ocpp_device", "protocol",0,24,true,enmDataTypeString,"ocpp1.6");
  append( "ocpp_device", "auth_cache_path",0,1024,true,enmDataTypeString,"/Settings/Auth.json");
  append( "ocpp_device", "firmwareVersion", 0, 24, true, enmDataTypeString, "2.0" );
  append( "ocpp_device", "iccid", 0, 24, true, enmDataTypeString, "XX" );
  append( "ocpp_device", "imsi", 0, 24, true, enmDataTypeString, "XX" );
  append( "ocpp_device", "meterSerialNumber", 0, 24, true, enmDataTypeString, "SELEC-1PH" );
  append( "ocpp_device", "meterType", 0, 24, true, enmDataTypeString, "XX" );
  append( "ocpp_device", "local_list_path", 0, 1024, true, enmDataTypeString, "/Settings/LocalList.json" );
  append( "ocpp_device", "offline_msg_queue_path", 0, 1024, true, enmDataTypeString, "/Settings/TXMsgs123.Q");
  append( "ocpp_device", "session_info_path", 0, 1024, true, enmDataTypeString, "/Settings/");
  append( "ocpp_params", "AllowOfflineTxForUnknownId", 0, 1, false, enmDataTypeBool, "false" );
  append( "ocpp_params", "AuthorizationCacheEnabled", 0, 1, false, enmDataTypeBool, "true" );
  append( "ocpp_params", "AuthorizeRemoteTxRequests", 0, 1, false, enmDataTypeBool, "true" );
  append( "ocpp_params", "BlinkRepeat", 0, 10, false, enmDataTypeInt, "5" );
  append( "ocpp_params", "ChargeProfileMaxStackLevel", 0, 1, true, enmDataTypeInt, "1" );
  append( "ocpp_params", "ChargingScheduleAllowedChargingRateUnit", 0, 32, true, enmDataTypeString, "1.0" );
  append( "ocpp_params", "ChargingScheduleMaxPeriods", 0, 1, true, enmDataTypeInt, "1" );
  append( "ocpp_params", "ClockAlignedDataInterval", 0, 900, false, enmDataTypeInt, "900" );
  append( "ocpp_params", "ConnectionTimeOut", 1, 120, false, enmDataTypeInt, "120" );
  append( "ocpp_params", "ConnectorPhaseRotation", 0, 1, false, enmDataTypeString, "1" );
  append( "ocpp_params", "TransactionMessageAttempts", 0, 1, false, enmDataTypeInt, "1" );
  append( "ocpp_params", "TransactionMessageRetryInterval", 0, 1, false, enmDataTypeInt, "1" );
  append( "ocpp_params", "ConnectorPhaseRotationMaxLength", 0, 1, true, enmDataTypeBool, "true" );
  append( "ocpp_params", "ConnectorSwitch3to1PhaseSupported", 0, 1, true, enmDataTypeBool, "true" );
  append( "ocpp_params", "GetConfigurationMaxKeys", 0, 100, true, enmDataTypeInt, "100" );
  append( "ocpp_params", "HeartbeatInterval", 1, 600, false, enmDataTypeInt, "10" );
  append( "ocpp_params", "LightIntensity", 1, 100, false, enmDataTypeInt, "100" );
  append( "ocpp_params", "LocalAuthListEnabled", 0, 1, false, enmDataTypeBool, "true" );
  append( "ocpp_params", "LocalAuthListMaxLength", 0, 1, true, enmDataTypeInt, "1" );
  append( "ocpp_params", "LocalAuthorizeOffline", 0, 1, false, enmDataTypeBool, "true" );
  append( "ocpp_params", "LocalPreAuthorize", 0, 1, false, enmDataTypeBool, "true" );
  append( "ocpp_params", "MaxChargingProfilesInstalled", 0, 10, true, enmDataTypeInt, "1" );
  append( "ocpp_params", "WebSocketPingInterval", 1, 3600, false, enmDataTypeInt, "45" );
  append( "ocpp_params", "MaxEnergyOnInvalidId", 0, 2000, false, enmDataTypeInt, "2000" );
  append( "ocpp_params", "MeterValueSampleInterval", 0, 100, false, enmDataTypeInt, "30" );
  append( "ocpp_params", "MeterValuesAlignedData", 0, 1, false, enmDataTypeString, "1" );
  append( "ocpp_params", "MeterValuesAlignedDataMaxlength", 0, 5, true, enmDataTypeString, "5" );
  append( "ocpp_params", "MeterValuesSampledData", 0, 1, false, enmDataTypeString, "1" );
  append( "ocpp_params", "MeterValuesSampledDataMaxLength", 0, 5, true, enmDataTypeInt, "5" );
  append( "ocpp_params", "MinimumStatusDuration", 0, 50, false, enmDataTypeInt, "30" );
  append( "ocpp_params", "NumberOfConnectors", 1, 5, true, enmDataTypeInt, "1" );
  append( "ocpp_params", "ReserveConnectorZeroSupported", 0, 1, true, enmDataTypeBool, "true" );
  append( "ocpp_params", "ResetRetries", 1, 5, true, enmDataTypeInt, "5" );
  append( "ocpp_params", "SendLocalListMaxLength", 0, 1, true, enmDataTypeInt, "1" );
  append( "ocpp_params", "UnlockConnectorOnEVSideDisconnect", 0, 1, true, enmDataTypeBool, "true" );
  append( "ocpp_params", "StatusbeatInterval", 0, 1, true, enmDataTypeBool, "true" );
  append( "ocpp_params", "StopTransactionOnEVSideDisconnect", 0, 1, false, enmDataTypeBool, "true" );
  append( "ocpp_params", "StopTransactionOnInvalidId", 0, 1, false, enmDataTypeBool, "true" );
  append( "ocpp_params", "StopTxnAlignedData", 0, 128, true, enmDataTypeString, "Energy.Active.Import" );
  append( "ocpp_params", "StopTxnAlignedDataMaxLength", 0, 128, true, enmDataTypeInt, "128" );
  append( "ocpp_params", "StopTxnSampleData", 0, 128, true, enmDataTypeString, "Energy.Active.Import" );
  append( "ocpp_params", "StopTxnSampleDataMaxLength", 0, 128, true, enmDataTypeInt, "128" );
  append( "ocpp_params", "SupportedFeatureProfiles", 0, 1, true, enmDataTypeString, "true" );
  append( "ocpp_params", "SupportedFeatureProfilesMaxLength", 0, 1, true, enmDataTypeBool, "true" );
}

  /* global settings file */
ZZSettings *gpcSettings = nullptr;

void initSettings( )
{
  gpcSettings = new ZZSettings;
  gpcSettings->load( );
}
