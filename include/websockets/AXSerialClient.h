
#ifndef AXSERIALCLIENT_H
#define AXSERIALCLIENT_H

#include <Arduino.h>

#define AX_CONNECTION_ESTABLISHED   ( 0x01 )
#define AX_CONNECTION_CLOSED        ( 0x00 )
#define AX_UART_HEADER_SIZE         ( 0x05 )
#define AX_MAX_BUFFER_SIZE          ( 64 )
#define AX_BUFFER_1K_SIZE           ( 1024 )
#define AX_MAX_UART_SIZE            ( AX_MAX_BUFFER_SIZE - AX_UART_HEADER_SIZE )

#define AX_TIMEOUT_MS              ( 5000 )
#define AX_CUSTOM_COMMAND

class AXSerialClient
{

public:

  AXSerialClient( void );

  /* Public Methods */
  void begin( Stream &port );
  void stop( void );
  void flush( void );

  void setTimeout( unsigned long timeout );

  int connect( const char *host, 
               uint16_t port );
  uint8_t connected( void );

  int available( void );

  int read( void ); 
  size_t read( uint8_t *buf, 
               size_t size 
             );
  size_t readBytes( char *buffer, 
                    size_t length 
                  );

  size_t write( const uint8_t *buf, 
                size_t size 
              );

    /* Inline Methods */
  inline size_t read( char *buffer,
                      size_t size 
                    )
  { 
    return read( (uint8_t *)buffer, size );
  }

  inline size_t write( char *buffer, 
                       size_t size
                     )
  {
    return write( (uint8_t *)buffer, size );
  }

  bool isTimeOut( void );
  bool hasValidCommandTime( void );

private:

  /* Private members */
  Stream *m_pSerial;

  uint8_t       m_connected;
  uint16_t      m_num_bytes_available;

  unsigned long m_lastCmdTimestamp;
  char m_payload_ch[AX_BUFFER_1K_SIZE];

  /* Private Method */
  void prepare_command( uint8_t cmd_u8, 
                        const char* payload_pcch, 
                        uint8_t input_len_u8,
                        char* out_pch,
                        uint8_t *output_len_u8
                     );

  void uart_read( void );

  size_t uart_write( uint8_t cmd_u8, 
                     const char* msg_pcch, 
                     size_t size 
                   );

  void uart_process_rx( const char* msg_pcch, 
                        uint8_t len_u8,
                        char* msg_out_pch, 
                        uint8_t *len_out_u8
                      );

  bool verify_chksum( const char* msg_pcc, 
                      uint8_t len_u8 
                    );

  uint8_t calc_chksum( const char* msg_pcch, 
                       uint8_t len_u8 
                     );
  void logMsg( const char* msg_pcch, uint16_t len_u16 );
};

#endif /* AXSERIALCLIENT_H */
