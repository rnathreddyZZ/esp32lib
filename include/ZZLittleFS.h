#ifndef ZZ_LITTLE_FS_H
#define ZZ_LITTLE_FS_H

  /* include the standard LittleFS */
#include <LittleFS.h>

  /* include debug macros */
#include <ZZDebug.h>

  /* function to save the given data of length to 
        the given filename */
void saveFile( const char* pcchFName, /* name of the file to be saved */
               const char* pchData,   /* data to be saved */
               int32_t iLen );        /* length of the data */

  /* function to read/retrieve the data from the
        given filename */
void getFile ( const char* pcchFName,   /* name of the file to retrieve */
               char** pchData,          /* data to be received */
               int32_t *piLen );        /* length of the data received */

  /* initialize the filesystem and optionally create
     settings file, if not exist*/
void initLittleFS( void );

#endif /* ZZ_LITTLE_FS_H */
