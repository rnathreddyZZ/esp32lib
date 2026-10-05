#ifndef ZZ_LITTLE_FS_H
#define ZZ_LITTLE_FS_H

  /* include the standard LittleFS */
#include <LittleFS.h>

  /* include debug macros */
#include <ZZDebug.h>

  /* function to save the given data of length to
        the given filename. returns true on success */
bool saveFile( const char* pcchFName, /* name of the file to be saved */
               const char* pchData,   /* data to be saved */
               size_t uLen );         /* length of the data */

  /* function to read/retrieve the data from the
        given filename into the caller's buffer. the data is
        always NUL terminated, so at most uBufSize - 1 bytes are read.
        returns false if the file can not be opened or does not fit */
bool getFile ( const char* pcchFName,   /* name of the file to retrieve */
               char* pchBuf,            /* buffer to receive the data */
               size_t uBufSize,         /* size of the buffer in bytes */
               size_t *puLen );         /* length of the data received */

  /* mount the filesystem (formatting it if it can not be
     mounted) and create the settings folder, if not exist.
     returns false if the filesystem is not usable */
bool initLittleFS( void );

#endif /* ZZ_LITTLE_FS_H */
