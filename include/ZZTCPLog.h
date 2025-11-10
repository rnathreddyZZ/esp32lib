#ifndef _ZZ_LOG_CLIENT_H_
#define _ZZ_LOG_CLIENT_H_

#include <ZZAsyncTCP.h>
#include <ZZDebug.h>

//#define ENABLE_TCP_LOGGING

class ZZTCPLog 
{

  public:
             ZZTCPLog (std::string strHost, uint16_t usPort);
    virtual ~ZZTCPLog ();

  void sendData (const char* pcchMsg);

private:

  std::string  m_strHost;
  uint16_t     m_usPort;

  boolean      m_bConnected,
               m_bConnecting;
  uint64_t     m_lLastDisconnected;

  ZZAsyncClient *m_pcSocket;

  void onConnect (void *arg, ZZAsyncClient *pcClnt);

  void onDisconnect (void *arg, ZZAsyncClient *pcClnt);

  void onReceive (void *arg, ZZAsyncClient *client,
                              void *data, size_t len);
};

void initLogger (const char* pcchAddress, uint16_t usPort);

void logMsg (const char* pcchMsg);

#endif // _ZZ_TCP_CLIENT_H_
