#ifndef _AX_JSON_IF_H_
#define _AX_JSON_IF_H_

#include "json.h"
#include <stdint.h>

class AXJSONIF
{

public:

  explicit AXJSONIF ();
  virtual ~AXJSONIF ();

  virtual void add_int (json_object *j, const char *name, int i) = 0;
  virtual void add_string (json_object *j, const char *name, const char *str) = 0;
  virtual void add_double (json_object *j, char *name, double d) = 0;

  virtual void get_string(json_object *j, const char *name,
                                   char *str, int size) = 0;

  virtual void get_string (json_object *j, char *str, int size) = 0;

  virtual void get_int (json_object *j, const char *name, int *i) = 0;

  virtual void get_double (json_object *j,char *name,double *f) = 0;

  virtual void get_json_obj (json_object *j, const char *name, json_object*& pcObj) = 0;

  virtual uint8_t get_length (json_object *j) = 0;

  virtual void get_json (json_object *j,
                  uint8_t ucIndex, json_object*& pcObj) = 0;

  virtual void clear (json_object *&pcObj) = 0;
};

#endif // _AX_JSON_IF_H_
