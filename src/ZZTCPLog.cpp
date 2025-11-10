
#include <ZZTCPLog.h>

ZZTCPLog::ZZTCPLog (std::string strHost, uint16_t usPort) :
                 m_strHost (std::move(strHost)), m_usPort (usPort),
                m_bConnected(false), m_bConnecting (false),
                m_lLastDisconnected (0)
{

  m_pcSocket = new ZZAsyncClient ();

  auto ocCB = [this](void* pcArg, ZZAsyncClient* pcClnt)
         { this->onConnect(pcArg, pcClnt); };
  m_pcSocket->onConnect (ocCB);

  auto ocDataCB = [this](void* pcArg, ZZAsyncClient* pcClnt, void *data, size_t len)
         { this->onReceive (pcArg, pcClnt, data, len); };
  m_pcSocket->onData (ocDataCB);

  auto ocDCB = [this](void* pcArg, ZZAsyncClient* pcClnt)
         { this->onDisconnect (pcArg, pcClnt); };
  m_pcSocket->onDisconnect (ocDCB);

  m_pcSocket->connect (m_strHost.c_str(), m_usPort);
  m_bConnecting = true;
  ZZ_DBG_INFO ("Connecting to %s (%d)...\n", m_strHost.c_str(), m_usPort);
}

ZZTCPLog::~ZZTCPLog ()
  { }

void ZZTCPLog::onConnect (void *arg, ZZAsyncClient *pcClnt)
{
	
  m_bConnected = true; m_bConnecting = false;
  ZZ_DBG_INFO ("TCP logging task started(%s on port %d)\n",
                                         m_strHost.c_str(), m_usPort);
}

void ZZTCPLog::onDisconnect (void *arg, ZZAsyncClient *pcClnt)
{
  
  m_bConnected = false; m_bConnecting = false;
  m_lLastDisconnected = millis ();
  ZZ_DBG_INFO ("TCP logging task stopped\n"); 
}

void ZZTCPLog::onReceive (void *arg, ZZAsyncClient *client,
                                    void *data, size_t len)
{

	ZZ_SERIAL.printf ("data received from %s \n", 
                         client->remoteIP().toString().c_str());
}

void ZZTCPLog::sendData (const char* pcchMsg)
{
 
  if (true == m_bConnected) {

    if (m_pcSocket->space() > strlen(pcchMsg) && m_pcSocket->canSend()) 
      { m_pcSocket->add (pcchMsg, strlen(pcchMsg)); m_pcSocket->send(); }
  }
  else
  {

    if (5000 < (millis () - m_lLastDisconnected))
    { 
      if (false == m_bConnecting)
      {

        m_pcSocket->connect (m_strHost.c_str(), m_usPort); 
        m_bConnecting = true; 
      }
    }

    DBG_PORT.print (pcchMsg);
  }    
}

static ZZTCPLog *gpcLog = nullptr;
void logMsg (const char* pcchMsg)
{

  if (nullptr != gpcLog)
    { gpcLog->sendData (pcchMsg); }
  else 
    { DBG_PORT.print (pcchMsg); }
}

  /* function to initialize the logger */
void initLogger (const char* pcchAddress, uint16_t usPort)
{

  if (NULL != pcchAddress && 0 < usPort)
    { gpcLog = new ZZTCPLog (pcchAddress, usPort); setLogMsgFunc(logMsg); }
}
