#ifndef ZZ_LITTLE_FS_H
#define ZZ_LITTLE_FS_H

  /* mount the filesystem (formatting it only if its content
     is corrupt) and create the settings folder, if not exist.
     returns false if settings can not be stored: filesystem
     not mounted, or settings folder could not be created */
bool initLittleFS( void );

#endif /* ZZ_LITTLE_FS_H */
