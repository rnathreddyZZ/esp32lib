
#include <json/AXJSON.h>
#include <ZZDebug.h>
#include <string.h>
#include <stdint.h>

AXJSON gostJSON;

AXJSON::AXJSON(bool bArray) : m_bArray (bArray)
  { }

AXJSON::~AXJSON ()
{ }

void AXJSON::newObj()
{ 

  if (m_bArray)
    { m_pcObj = json_object_new_array (); }
  else
    { m_pcObj = json_object_new_object();  }
}

void AXJSON::clear()
  { json_object_put(m_pcObj); m_pcObj = nullptr; }

json_object* AXJSON::get_json_obj ()
  { return m_pcObj; }

void AXJSON::add (const char* pcchName, json_object *pcObj)
{

  if (m_bArray)
    { json_object_array_add (m_pcObj, pcObj); }
  else
    { json_object_object_add (m_pcObj, pcchName, pcObj); }
}

void AXJSON::add_int (const char *name, int i)
{

  json_object *pcObj;

  pcObj = json_object_new_int (i);
  json_object_object_add (m_pcObj, name, pcObj);
}

void AXJSON::add_string (const char *name, const char *str)
{

  json_object *pcObj;

  pcObj = json_object_new_string (str);
  json_object_object_add (m_pcObj, name, pcObj);
}

void AXJSON::add_double (char *name, double d)
{

  json_object *pcObj;

  pcObj = json_object_new_double (d);
  json_object_object_add (m_pcObj, name, pcObj);
}

void AXJSON::get_string_ex (const char* parent, const char* name,
                                                char* str, int size)
{

  json_object *pcValue, *pcParent;
  
  *str=0;

  json_object_object_get_ex (m_pcObj, parent, &pcParent);
  if (pcParent) {

    json_object_object_get_ex (pcParent, name, &pcValue);
    if (pcValue)
      { strncpy (str, json_object_get_string (pcValue), size); }
  }
}

void AXJSON::get_string_ex (uint8_t ucInd, const char* name,
                                                char* str, int size)
{

  json_object *pcValue, *pcParent;
  
  *str=0;

  pcParent  = json_object_array_get_idx(m_pcObj, ucInd);
  if (pcParent) {

    json_object_object_get_ex (pcParent, name, &pcValue);
    if (pcValue)
      { strncpy (str, json_object_get_string (pcValue), size); }
  }
}

void AXJSON::get_string (const char *name, char *str, int size)
{

  json_object *pcValue;

  json_object_object_get_ex (m_pcObj, name, &pcValue);
  if (pcValue)
    { strncpy (str, json_object_get_string (pcValue), size); }
  else
  { *str=0; }
}

void AXJSON::get_string (char *str, int size)
  { strncpy (str, json_object_get_string (m_pcObj), size); }

void AXJSON::get_int_ex (uint8_t ucInd, const char* name, int *i)
{

  json_object *pcRObj = json_object_array_get_idx(m_pcObj, ucInd);
  json_object *pcValue;

  json_object_object_get_ex (pcRObj, name, &pcValue);

  if (pcValue)
    { *i = json_object_get_int (pcValue);  }
  else
    { *i=0; }
}

void AXJSON::get_int(const char *name,int *i)
{

  json_object *pcValue;
  json_object_object_get_ex (m_pcObj, name, &pcValue);

  if (pcValue)
    { *i = json_object_get_int (pcValue);  }
  else
    { *i=0; }
}

void AXJSON::get_double (char *name,double *f)
{

  json_object *pcValue;

  json_object_object_get_ex (m_pcObj, name, &pcValue);

  if (pcValue)
    { *f = json_object_get_double(pcValue);  }
  else
    { *f=0; }
}

void AXJSON::get_json_obj (const char *name,
                            json_object*& pcObj)
  { json_object_object_get_ex (m_pcObj, name, &pcObj); }

uint8_t AXJSON::get_length ()
  { return json_object_array_length (m_pcObj); }


void AXJSON::get_json (uint8_t ucIndex,
                       json_object*& pcObj)
{ pcObj = json_object_array_get_idx(m_pcObj, ucIndex); }

bool AXJSON::init(const char *pcchData)
{

  m_pcObj = json_tokener_parse (pcchData);

  return (NULL != m_pcObj);
}

const char *AXJSON::get()
{ return json_object_to_json_string(m_pcObj); }

const char *AXJSON::getKey()
{

  const char* pcchKey = NULL;

  #if defined(ESP32)
    if (NULL != m_pcObj) {

      struct lh_entry *pstSt = lh_table_head(json_object_get_object(m_pcObj));
      pcchKey = (const char *)lh_entry_k(pstSt);
    }
  #endif // ESP32

  return pcchKey;
}

std::vector<const char*> AXJSON::getKeys ()
{

  std::vector<const char*> ppcParams;
  json_object *pcSec = nullptr,
              *pcParam = nullptr;
  
  const char* pcchSec = getKey ();

  json_object_object_get_ex (m_pcObj, pcchSec, &pcSec);
  uint8_t ucLen = json_object_array_length (pcSec);
  for (uint8_t ucInd = 0; ucInd < ucLen; ucInd++) {

    json_object *pcC = json_object_array_get_idx (pcSec, ucInd);
    struct lh_entry *pstSt = lh_table_head(json_object_get_object(pcC));
    const char* pcchParam = (const char *)lh_entry_k(pstSt);
    ppcParams.push_back (pcchParam);
  }

  return ppcParams;
}

json_object *AXJSON::getParam (const char* pcchSecIn, 
                                 const char* pcchParamIn)
{

  json_object *pcSec = nullptr,
              *pcParam = nullptr;
  const char* pcchSec = getKey ();

  if (nullptr != pcchSec && strcmp (pcchSec, pcchSecIn) == 0) {

    json_object_object_get_ex (m_pcObj, pcchSec, &pcSec);
    uint8_t ucLen = json_object_array_length (pcSec);
    for (uint8_t ucInd = 0; ucInd < ucLen; ucInd++) {

      json_object *pcC = json_object_array_get_idx (pcSec, ucInd);
      struct lh_entry *pstSt = lh_table_head(json_object_get_object(pcC));
      const char* pcchParam = (const char *)lh_entry_k(pstSt);      
      if (0 == strcmp (pcchParam, pcchParamIn))
      {
       json_object_object_get_ex (pcC, pcchParam, &pcParam);
       if (nullptr != pcParam)
         { break; }
      }
    }
  }

  return pcParam;
}

void AXJSON::setParam (const char* pcchSecIn, 
                               const char* pcchParamIn, 
                                  const char* pcchKeyIn,
                                   const char* pcchValue)
{

  json_object *pcValue = nullptr,
              *pcParam = getParam (pcchSecIn, pcchParamIn);

  if (pcParam) {

    json_object_object_get_ex (pcParam, pcchKeyIn, &pcValue);
    if (pcValue) 
    {
      json_object_set_string (pcValue, pcchValue);
    }
  }
}

const char* AXJSON::getParam (const char* pcchSecIn, 
                               const char* pcchParamIn, 
                                  const char* pcchKeyIn)
{

  const char *pcchValue { nullptr }; 

  json_object *pcValue = nullptr,
              *pcParam = getParam (pcchSecIn, pcchParamIn);

  if (pcParam) {

    json_object_object_get_ex (pcParam, pcchKeyIn, &pcValue);
    if (pcValue)
      { pcchValue = json_object_get_string (pcValue); }
  }

  return pcchValue;
}

bool AXJSON::setIntParam(const char* pcchSec,
                          const char* pcchParam,
                            const  char* pcchKey, 
                              const int32_t iValue)
{
  
  bool bRCode = false;
 
  json_object *pcValue = nullptr,
              *pcParam = getParam (pcchSec, pcchParam);

  if (pcParam) {

    json_object_object_get_ex (pcParam, pcchKey, &pcValue);
    if (pcValue) {
       if (json_type_int == json_object_get_type(pcValue))
       {
         int iV =  json_object_set_int (pcValue, iValue); 
         bRCode = true;
       } 
    }
  }

  return bRCode;
}

bool AXJSON::getIntParam(const char* pcchSec,
                          const char* pcchParam,
                            const  char* pcchKey, 
                                  int32_t& iValue)
{
  
  bool bRCode = false;
 
  json_object *pcValue = nullptr,
              *pcParam = getParam (pcchSec, pcchParam);

  iValue = 0;
  if (pcParam) {

    json_object_object_get_ex (pcParam, pcchKey, &pcValue);
    if (pcValue) {
       if (json_type_int == json_object_get_type(pcValue))
       {
         iValue = json_object_get_int (pcValue);
         bRCode = true;
       } 
    }
  }

  return bRCode;
}

bool AXJSON::setBoolParam(const char* pcchSec,
                           const char* pcchParam,
                              const  char* pcchKey, 
                                const bool bValue)
{
 
  bool bRCode = false;

  json_object *pcValue = nullptr,
              *pcParam = getParam (pcchSec, pcchParam);

  if (pcParam) {

    json_object_object_get_ex (pcParam, pcchKey, &pcValue);
    if (pcValue) 
    {
      if (json_type_boolean == json_object_get_type(pcValue))
      {
        json_object_set_boolean (pcValue, bValue); 
        bRCode = true;
      }
    }
  }

  return bRCode;
}

bool AXJSON::getBoolParam(const char* pcchSec,
                           const char* pcchParam,
                              const  char* pcchKey, 
                                       bool& bValue)
{
 
  bool bRCode = false;

  json_object *pcValue = nullptr,
              *pcParam = getParam (pcchSec, pcchParam);

  bValue = false;
  if (pcParam) {

    json_object_object_get_ex (pcParam, pcchKey, &pcValue);
    if (pcValue) {

       if (json_type_boolean == json_object_get_type(pcValue))
       {
         bValue = json_object_get_boolean (pcValue);
         bRCode = true;
       } 
    }
  }

  return bRCode;
}
