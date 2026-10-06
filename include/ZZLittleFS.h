#ifndef ZZ_LITTLE_FS_H
#define ZZ_LITTLE_FS_H

  /* mount the filesystem (formatting it if it can not be
     mounted) and create the settings folder, if not exist.
     returns false if the filesystem is not usable */
bool initLittleFS( void );

#endif /* ZZ_LITTLE_FS_H */
