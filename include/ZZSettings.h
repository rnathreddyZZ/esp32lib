#ifndef ZZ_SETTINGS_H
#define ZZ_SETTINGS_H

#include <cstdint>
#include <string>
#include <cstdio>
#include <vector>
#include <map>

typedef enum data_type_enm
{
  enmDataTypeNone     = 0x00,
  enmDataTypeBool     = 0x01,
  enmDataTypeInt      = 0x02,
  enmDataTypeDouble   = 0x03,
  enmDataTypeString   = 0x04,
}enmDataType;

typedef struct stparam
{
  std::string  name_str;
  uint16_t     min_u16;
  uint16_t     max_u16;
  bool         readonly_b;
  enmDataType  type_u8;
  std::string  value_str;
}stParam;

typedef std::vector<stParam> stParams;
typedef std::vector<stParam>::iterator stParamIter;

class ZZSettings
{

public:
           ZZSettings( );
  virtual ~ZZSettings( );

  void init( void );

  void dump( void );

  void save( void );

  void load( void );

  void append( const char     *section_pch,
               const char     *name_pch,
               const uint16_t  min_u16,
               const uint16_t  max_u16,
               const bool      readonly_b,
               const uint8_t   type_u8,
               const char      *value_pch );

    /* false if the parameter does not exist or the value does
       not match its type and range */
  bool setParam( const char  *section_pch,
                 const char *name_pch, 
                 const char *value_pch );

  bool setParam( const char  *section_pch,
                 const char *name_pch, 
                 int32_t value_i32 );


  bool getParam( const char* sec_pch, 
                 const char *name_pch,
                 char* value_pch );

  bool getBoolParam( const char* sec_pch, 
                     const char *name_pch,
                     bool& value_b );

  bool getIntParam( const char* sec_pch, 
                    const char *name_pch, 
                    uint32_t& value_u32 );

  bool getDblParam( const char* sec_pch, 
                    const char *name_pch,
                    double& value_d );

  bool isReadOnly( const char* sec_pch,
                   const char* name_pch );

  bool getRangeByParam( const char* sec_pch,
                        const char *name_pch,
                        const char *value_pcch,
                        uint16_t& value_u16 );

  enmDataType getType( const char* sec_pcch, 
                       const char* name_pcch );

  std::vector<std::string> getSections( );

  std::vector<std::string> getKeys( const char* sec_pcch );

  std::string getData( std::string sec_str );

    /* all-or-nothing: false, and nothing changed, if any entry
       is unknown, read-only or invalid */
  bool setData( std::string data_str ); 

private:

  std::map<std::string, std::vector<stParam>> m_section_ost;

  stParam* findParam( const char* sec_pch,
                      const char* name_pch );

  bool isValidValue( const stParam& param_ost,
                     const char* value_pch );
};

void initSettings( void );

extern ZZSettings *gpcSettings;

#define ZZ_GET_PARAM( sec_pcch, name_pcch, value_pch ) \
           gpcSettings->getParam( sec_pcch, name_pcch, value_pch )

#define ZZ_GET_BOOL_PARAM( sec_pcch, name_pcch, value_b ) \
           gpcSettings->getBoolParam( sec_pcch, name_pcch, value_b )

#define ZZ_GET_INT_PARAM( sec_pcch, name_pcch, value_u32 ) \
          gpcSettings->getIntParam( sec_pcch, name_pcch, value_u32 )

#define ZZ_GET_DOUBLE_PARAM( sec_pcch, name_pcch, value_d ) \
           gpcSettings->getDblParam( sec_pcch, name_pcch, value_d )

#endif /* ZZ_SETTINGS_H */
