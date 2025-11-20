#include "ZZDebug.h"
#include <websockets/AXSerialClient.h>
#include <ZZSettings.h>
uint8_t data_call_status_u8 = false;
void axtohex( const uint8_t *msg_pu8,
              uint8_t len_u8,
              char* msg_pch
            );

  /* Constructor */
AXSerialClient::AXSerialClient( void )
                : m_connected( AX_CONNECTION_CLOSED ),
                  m_pSerial( nullptr ),
                  m_lastCmdTimestamp( 0 ),
                  m_num_bytes_available( 0 )
{

  Serial2.begin( 115200, SERIAL_8N1, 42, 41 );
  begin( Serial2 );
}

/* Public Methods */

void AXSerialClient::begin( Stream &port )
{

  port.setTimeout( 100 );
  m_pSerial = &port;
  m_connected = AX_CONNECTION_ESTABLISHED;
}

void AXSerialClient::stop( void )
{
  m_connected = AX_CONNECTION_CLOSED;
  ZZ_DBG_INFO("client disconnected\n");

  uart_write( 0x05, "", 1 );
}

void AXSerialClient::flush( void )
{
  if( m_pSerial != nullptr )
  {
    m_pSerial->flush( );
  }
}

void AXSerialClient::setTimeout( unsigned long timeout )
{
  if( m_pSerial != nullptr )
  {
    m_pSerial->setTimeout( timeout );
  }
}

int AXSerialClient::connect( const char *host,
                             uint16_t port
                           )
{
  if(data_call_status_u8 == false)
  {
    char port_buff_ch[AX_MAX_BUFFER_SIZE] = { '\0', };
    char apn_name[254];

    ZZ_GET_PARAM ("application", "APNName", apn_name);
    ZZ_DBG_INFO( "APN name: %s\n", apn_name );

    // Send APN Name to modem
    uart_write( 0x01, apn_name, strlen(apn_name));
    delay( 10 );

    // Send Host to modem
    uart_write( 0x02, host, strlen(host));
    delay( 10 ); // Small delay to allow the modem to process

    // Send Port to modem
    sprintf( port_buff_ch, "%d", port );
    uart_write( 0x03, port_buff_ch, strlen(port_buff_ch) );
    delay( 5000 );

    if( m_pSerial->available( ) )
    {
      char status_buf_sz[1024];
      read( status_buf_sz, 7 );

      if( strcmp(status_buf_sz,"OK") == 0 )
      {
        data_call_status_u8 = true;
      }
      ZZ_DBG_INFO("Data Call Status:%d %s\n", data_call_status_u8, status_buf_sz);
    }
  }
  return data_call_status_u8;
}

uint8_t AXSerialClient::connected( void )
{
  return m_connected;
}

int AXSerialClient::available( void )
{

  if( m_pSerial != nullptr )
  {
    if(!m_num_bytes_available)
    {
      return m_pSerial->available( );
    }
    else
    {
      return m_num_bytes_available;
    }
  }

  return 0; // Return 0 if not initialized
}

int AXSerialClient::read( void )
{

  if( m_pSerial != nullptr )
  {
    return m_pSerial->read( );
  }

  return -1; // Return -1 if not initialized
}

size_t AXSerialClient::read( uint8_t *buffer,
                             size_t size
                           )
{

  char buffer_sz[AX_MAX_BUFFER_SIZE];
  uint8_t buffer_u8 = 0,
          len_out_u8 = 0;
  uint16_t max_len_u16 = AX_BUFFER_1K_SIZE - m_num_bytes_available;
  ZZ_DBG_DEBUG( "Read Bytes: %d %d\n", m_num_bytes_available, size );
  if( m_pSerial != nullptr && (max_len_u16 <= AX_BUFFER_1K_SIZE) && (m_num_bytes_available < size))
  {
    do
    {
      uint8_t bytes_to_be_read_u8 = (max_len_u16 > AX_MAX_BUFFER_SIZE) ?
                                                  AX_MAX_BUFFER_SIZE : max_len_u16;
      ZZ_DBG_DEBUG( "Bytes to be read: (%d of %d)\n", bytes_to_be_read_u8, max_len_u16 );
      buffer_u8 = m_pSerial->readBytes( buffer_sz, bytes_to_be_read_u8 );
      if( 0 < buffer_u8 )
      {
        len_out_u8 = 0;
        uart_process_rx( buffer_sz, buffer_u8,
                          (char*)(m_payload_ch + m_num_bytes_available), &len_out_u8 );
        m_num_bytes_available += len_out_u8;
        max_len_u16 -= len_out_u8;
        m_payload_ch[m_num_bytes_available] = '\0';
      }

      delay( 20 );
    }while( 0 < buffer_u8 && m_pSerial->available( ) != 0);
  }

  ZZ_DBG_DEBUG( "Bytes Available: \n" );
  // logMsg( m_payload_ch, size );

  size = (size >= m_num_bytes_available) ? m_num_bytes_available : size;
  memcpy( (char*)buffer, m_payload_ch, size );
  buffer[size] = '\0';

  m_num_bytes_available -= size;
  memcpy( (char*)m_payload_ch, m_payload_ch+size, m_num_bytes_available );

  ZZ_DBG_INFO( "Returned to Client: \n" );
  logMsg( (char*)buffer, size );

  return size;
}

size_t AXSerialClient::readBytes( char *buffer,
                                  size_t length
                                )
{
  if( m_pSerial != nullptr )
  {
    return read( (uint8_t*)buffer, length );
  }

  return 0;
}

size_t AXSerialClient::write( const uint8_t *buffer,
                              size_t size
                            )
{

  if( m_pSerial != nullptr )
  {
    return uart_write( 0x04, (const char*)buffer, size );
  }

  return 0;
}

bool AXSerialClient::isTimeOut( void )
{
  bool is_time_out = false;
  unsigned long current_millis_ul = millis( );

  if( m_lastCmdTimestamp != 0 &&
              current_millis_ul - m_lastCmdTimestamp > AX_TIMEOUT_MS )
  {
    is_time_out = true;
    ZZ_DBG_INFO( "Connection Timeout\n" );
  }
  return is_time_out;
}

bool AXSerialClient::hasValidCommandTime( void )
{
  return (m_lastCmdTimestamp != 0);
}

void AXSerialClient::prepare_command( uint8_t cmd_u8,
                                      const char* payload_pcch,
                                      uint8_t len_in_u8,
                                      char* out_pch,
                                      uint8_t *len_out_u8
                                    )
{

  uint8_t chksum_u8 = 0;

  out_pch[0] = 'a';
  out_pch[1] = 'x';
  out_pch[2] = len_in_u8;
  out_pch[3] = cmd_u8;

  memcpy( out_pch + 4, payload_pcch, len_in_u8 );

  chksum_u8 = calc_chksum( out_pch + 2, len_in_u8+2 );

  *len_out_u8 = len_in_u8 + 5; /* including the two bytes of the header */

  out_pch[(*len_out_u8)-1] = chksum_u8;
  ZZ_DBG_DEBUG("Writing Payload: %d bytes: %s\n", len_in_u8, payload_pcch);
}

void AXSerialClient::uart_read( void )
{

  uint8_t buff_u8[ AX_MAX_BUFFER_SIZE ];
  char buff_out_sz[ AX_MAX_BUFFER_SIZE ];
  size_t numByte = 0;
  uint8_t len_out_u8 = 0;

  memset( buff_u8, 0, AX_MAX_BUFFER_SIZE );
  numByte = read( (char*)buff_u8, AX_MAX_BUFFER_SIZE );

  uart_process_rx( (char*)buff_u8, numByte, buff_out_sz, &len_out_u8 );

  logMsg( buff_out_sz, len_out_u8 );
}

size_t AXSerialClient::uart_write( uint8_t cmd_u8, const char* msg_pcch, size_t length_t )
{

  size_t bytes_written = 0;

  char msg_out[ AX_MAX_BUFFER_SIZE ] = { '\0', };
  ZZ_DBG_INFO( "uart-write: (%d) %s\n", length_t, msg_pcch );
  while ( bytes_written < length_t )
  {
    size_t bytes_to_be_written = 0;
    uint8_t bytes_written_u8 = 0;

    bytes_to_be_written = (length_t - bytes_written) > AX_MAX_UART_SIZE ?
                                       AX_MAX_UART_SIZE : (length_t - bytes_written);
    memset( msg_out, 0x00, AX_MAX_BUFFER_SIZE );

#if defined( AX_CUSTOM_COMMAND )
    prepare_command( cmd_u8, msg_pcch+bytes_written,
                        (uint8_t)bytes_to_be_written, msg_out, &bytes_written_u8 );

    logMsg( msg_out, bytes_written_u8 );
#else
    bytes_written_u8 = length_t;
    memcpy(msg_out,msg_pcch,bytes_written_u8);
    logMsg( msg_out, bytes_written_u8 );
#endif /* AX_CUSTOM_COMMAND */

    if( bytes_written_u8 != m_pSerial->write( (uint8_t *)msg_out, bytes_written_u8 ))
    {
      ZZ_DBG_ERR( "Failed write to the UART\n" );
    }

    bytes_written += bytes_to_be_written;

    delay( 10 );
  }

  if(bytes_written != 0)
  {
    if (msg_pcch[1] == '3')
    {
      m_lastCmdTimestamp = 0;
      ZZ_DBG_INFO( "Reset m_lastCmdTimestamp to zero\n" );
    }
    else
    {
      ZZ_DBG_INFO( "Timestamp updated for the last command. %d %d, %c\n", bytes_written, length_t, msg_pcch[1]);
      m_lastCmdTimestamp = millis();
    }
  }

  return bytes_written;
}

void AXSerialClient::uart_process_rx( const char* msg_pcch, uint8_t len_u8,
                                      char* msg_out_pch, uint8_t *len_out_u8
                                    )
{

  if (len_u8 > 5)
  {
    if( verify_chksum( msg_pcch, len_u8 ) )
    {

      char buff_out_sz[AX_MAX_BUFFER_SIZE+1];
      memcpy(buff_out_sz, msg_pcch+4, len_u8 - 5);
      *len_out_u8 = len_u8 - 5;
      buff_out_sz[*len_out_u8] = '\0';
      memcpy(msg_out_pch, buff_out_sz, *len_out_u8);

      m_connected = AX_CONNECTION_ESTABLISHED;
      m_lastCmdTimestamp = 0;

      ZZ_DBG_DEBUG( "Payload received from UART: \n" );
      //logMsg( buff_out_sz, len_u8 );
    }
    else
    {
      ZZ_DBG_INFO( "Invalid Check Sum...!\n" );
      logMsg( msg_pcch, len_u8 );
    }
  }
}

bool AXSerialClient::verify_chksum( const char* msg_pcc, uint8_t len_in_u8 )
{

  bool retCode = false;

  uint8_t check_sum = 0x00;
  uint8_t check_sum_in = 0x00;

  uint8_t ind_u8 = 0x00;
  uint8_t len_u8 = 0x00;

    /* length execluding checksum */
  len_u8 = len_in_u8 - 1;


    /* incoming checksum */
  check_sum_in = msg_pcc[len_u8];

    /* iterate through the data and compute the checksum */
  for( ind_u8 = 0x02; ind_u8 < len_u8; ind_u8++ )
  {
      /* XOR for each byte */
    check_sum ^= msg_pcc[ind_u8];
  }

    /* is the checksum valid? */
  if( check_sum == check_sum_in )
  {
    /* checksum is valid */
    retCode = true;
  }

  return retCode;
}

uint8_t AXSerialClient::calc_chksum( const char* msg_pcch, uint8_t len_u8 )
{

  uint8_t chksum_u8 = 0;
  uint16_t ind_u16 = 0;

    /* iterate through the data and compute the checksum */
  for( ind_u16 = 0x00; ind_u16 < len_u8; ind_u16++ )
  {  /* XOR for each byte */
    chksum_u8 ^= msg_pcch[ind_u16];
  }

  return chksum_u8;
}

void AXSerialClient::logMsg( const char* msg_pcch, uint16_t len_u16 )
{
  char buff_ch[1024] = { '\0', };
  axtohex( (uint8_t*)msg_pcch, len_u16, buff_ch );
  ZZ_DBG_INFO( "(%d) %s\n%s\n", len_u16, buff_ch, msg_pcch );
}

void axtohex( const uint8_t *msg_pu8, uint8_t len_u8, char* msg_pch )
{

  uint16_t j = 0;
  uint8_t dec_u8 = 0;
  uint8_t ch = 0;

  for( uint8_t ind_u8 = 0; ind_u8 < len_u8; ind_u8++ )
  {
    dec_u8 = msg_pu8[ind_u8];
    ch = 0;

    if( dec_u8 == 0 )
    {
      msg_pch[j++] = 0;
      msg_pch[j++] = 0;
    }
    else {

      ch = dec_u8 >> 4;
      msg_pch[j++] = ( ch < 10 ) ? ( ch + 48 ) : ( ch + 55 );

      ch = dec_u8 & 0x0F;
      msg_pch[j++] = ( ch < 10 ) ? ( ch + 48 ) : ( ch + 55 );
    }
  }

  msg_pch[j] = '\0';
}
