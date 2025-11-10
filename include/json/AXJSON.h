#ifndef _AX_JSON_H
#define _AX_JSON_H

#include <vector>
#include "AXJSONIF.h"

class AXJSON
{

public:

  explicit AXJSON (bool bArray = false);
  virtual ~AXJSON ();

  void newObj();
  void clear ();

  void add_int (const char *name, int i);
  void add_string (const char *name, const char *str);
  void add_double (char *name, double d);
  void add (const char* pcchName, json_object *pcObj);

  void get_string_ex (const char* parent, const char* name,
                           char* str, int size);
                           
  void get_string_ex (uint8_t ucInd, const char* name,
                           char* str, int size);

  void get_string(const char *name,
                     char *str, int size);

  void get_string (char *str, int size);

  void get_int_ex (uint8_t ucInd, const char* name, int *i);
  void get_int (const char *name, int *i);

  void get_double (char *name,double *f);

  void get_json_obj (const char *name, json_object*& pcObj);
  json_object* get_json_obj ();

  uint8_t get_length ();

  void get_json (uint8_t ucIndex, json_object*& pcObj);

  bool init (const char* pcchData);

  const char *get ();

  const char *getKey ();

  json_object *getParam (const char* pcchSecIn, 
                          const char* pcchParamIn);

  void setParam (const char* pcchSecIn, 
                   const char* pcchParamIn, 
                     const char* pcchKeyIn,
                      const char* pcchValue);

  const char* getParam (const char* pcchSecIn, 
                          const char* pcchParamIn, 
                            const char* pcchKeyIn);

  bool setIntParam(const char* pcchSec,
                    const char* pcchParam,
                       const  char* pcchKey, 
                          const  int32_t iValue);

  bool getIntParam(const char* pcchSec,
                    const char* pcchParam,
                       const  char* pcchKey, 
                           int32_t& iValue);

  bool setBoolParam(const char* pcchSec,
                     const char* pcchParam,
                       const  char* pcchKey, 
                          const bool bValue);

  bool getBoolParam(const char* pcchSec,
                     const char* pcchParam,
                       const  char* pcchKey, 
                             bool& bValue);

  std::vector<const char*> getKeys ();

public:

  json_object *m_pcObj;

  bool m_bArray;
};

extern AXJSON gostJSON;

#endif // _AX_JSON_H
