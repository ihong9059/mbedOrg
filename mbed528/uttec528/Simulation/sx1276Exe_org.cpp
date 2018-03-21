#include "mbed.h"
#include "main.h"
#include "sx1276-hal.h"
#include "debug.h"
#include "sx1276Exe.h"

DigitalOut led(LED1);

volatile AppStates_t State = LOWPOWER;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*
 *  Global variables declarations
 */

SX1276MB1xAS Radio( NULL );

const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";

uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];

int16_t RssiValue = 0.0;
int8_t SnrValue = 0.0;

sxRxFrame_t m_sxRxFrame = {0,};

sx1276Exe::sx1276Exe(){
}

void sx1276Exe::sendLoRa(sxTxFrame_t sFrame){
    Radio.Send( sFrame.ptrBuf, sFrame.size );
//    printf("sendLoRa\n\r");
}
bool sx1276Exe::getRxFlag(){
    return m_sxRxFrame.rxFlag;
}
void sx1276Exe::clearRxFlag(){
    m_sxRxFrame.rxFlag=false;
}

sxRxFrame_t* sx1276Exe::readLoRa(){
    printf("readLoRa\n\r");
    Radio.Rx( RX_TIMEOUT_VALUE );
    return &m_sxRxFrame;
}
/*
*/
void sx1276Exe::initSx1276(){
    // Initialize Radio driver
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.RxError = OnRxError;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    Radio.Init( &RadioEvents );
    
    // verify the connection with the board
    while( Radio.Read( REG_VERSION ) == 0x00  )
    {
        debug( "Radio could not be detected!\n\r", NULL );
        wait( 1 );
    }
            
    debug_if( ( DEBUG_MESSAGE & ( Radio.DetectBoardType( ) == SX1276MB1LAS ) ) , "\n\r > Board Type: SX1276MB1LAS < \n\r" );
    debug_if( ( DEBUG_MESSAGE & ( Radio.DetectBoardType( ) == SX1276MB1MAS ) ) , "\n\r > Board Type: SX1276MB1MAS < \n\r" );
    
    Radio.SetChannel( RF_FREQUENCY ); 
    
    debug_if( LORA_FHSS_ENABLED, "\n\n\r             > LORA FHSS Mode < \n\n\r");
    debug_if( !LORA_FHSS_ENABLED, "\n\n\r             > LORA Mode < \n\n\r");
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                         LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                         LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                         LORA_CRC_ENABLED, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
                         LORA_IQ_INVERSION_ON, 2000000 );
    
    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                         LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                         LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON, 0,
                         LORA_CRC_ENABLED, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
                         LORA_IQ_INVERSION_ON, true );
     
    debug_if( DEBUG_MESSAGE, "\n\rStarting Ping-Pong loop\r\n" ); 
        
    led = 0;
        
    Radio.Rx( RX_TIMEOUT_VALUE );
    testSx1276();
//    testTxRx();
}

void sx1276Exe::testTxRx(uint32_t uiTest){
    uint8_t LoRaBuf[32];
    sxTxFrame_t sTxFrame;
//    printf("testTxRx:%d\n\r",uiTest);
    int iSize=sprintf((char*)LoRaBuf,"Count=%d\n\r",uiTest);
    sTxFrame.ptrBuf=LoRaBuf; sTxFrame.size=(uint8_t)iSize;            
    sendLoRa(sTxFrame);
}

void testSx1276(){
    uint8_t i;
    bool isMaster = true;
    while( 1 )
    {
        switch( State )
        {
        case RX:
            if( isMaster == true )
            {
                if( BufferSize > 0 )
                {
                    if( strncmp( ( const char* )Buffer, ( const char* )PongMsg, 4 ) == 0 )
                    {
                        led = !led;
                        debug( "RX Master...Pong\r\n" );
                        // Send the next PING frame            
    printf("\n\r");
    for(int i=0;i<BufferSize;i++) printf("%d,%d:",i,Buffer[i]);
    printf("\n\r");
                        strcpy( ( char* )Buffer, ( char* )PingMsg );
                        // We fill the buffer with numbers for the payload 
                        for( i = 4; i < BufferSize; i++ )
                        {
                            Buffer[i] = i - 4;
                        }
                        wait_ms( DeTxDelay ); 
                        Radio.Send( Buffer, BufferSize );
                    }
                    else if( strncmp( ( const char* )Buffer, ( const char* )PingMsg, 4 ) == 0 )
                    { // A master already exists then become a slave
                        debug( "RX Master...Ping\r\n" );
                        led = !led;
                        isMaster = false;
                        // Send the next PONG frame            
    printf("\n\r");
    for(int i=0;i<BufferSize;i++) printf("%d,%d:",i,Buffer[i]);
    printf("\n\r");
                        strcpy( ( char* )Buffer, ( char* )PongMsg );
                        // We fill the buffer with numbers for the payload 
                        for( i = 4; i < BufferSize; i++ )
                        {
                            Buffer[i] = i - 4;
                        }
                        wait_ms( DeTxDelay ); 
                        Radio.Send( Buffer, BufferSize );
                    }
                    else // valid reception but neither a PING or a PONG message
                    {    // Set device as master ans start again
                        isMaster = true;
                        Radio.Rx( RX_TIMEOUT_VALUE );
                    }    
                }
            }
            else
            {
                if( BufferSize > 0 )
                {
                    if( strncmp( ( const char* )Buffer, ( const char* )PingMsg, 4 ) == 0 )
                    {
                        led = !led;
                        debug( "RX Slave...Ping\r\n" );
                        // Send the reply to the PING string
                        strcpy( ( char* )Buffer, ( char* )PongMsg );
                        // We fill the buffer with numbers for the payload 
                        for( i = 4; i < BufferSize; i++ )
                        {
                            Buffer[i] = i - 4;
                        }
                        wait_ms( 10 );  
                        Radio.Send( Buffer, BufferSize );
                    }
                    else // valid reception but not a PING as expected
                    {    // Set device as master and start again
                        isMaster = true;
                        Radio.Rx( RX_TIMEOUT_VALUE );
                    }    
                }
            }
            State = LOWPOWER;
            break;
        case TX:    
            led = !led; 
            if( isMaster == true )  
            {
                debug( "Ping...\r\n" );
            }
            else
            {
                debug( "Pong...\r\n" );
            }
            Radio.Rx( RX_TIMEOUT_VALUE );
            State = LOWPOWER;
            break;
        case RX_TIMEOUT:
            if( isMaster == true )
            {
                // Send the next PING frame
                strcpy( ( char* )Buffer, ( char* )PingMsg );
                for( i = 4; i < BufferSize; i++ )
                {
                    Buffer[i] = i - 4;
                }
                wait_ms( 10 ); 
                Radio.Send( Buffer, BufferSize );
            }
            else
            {
                Radio.Rx( RX_TIMEOUT_VALUE );  
            }             
            State = LOWPOWER;
            break;
        case RX_ERROR:
            // We have received a Packet with a CRC error, send reply as if packet was correct
            if( isMaster == true )
            {
                // Send the next PING frame
                strcpy( ( char* )Buffer, ( char* )PingMsg );
                for( i = 4; i < BufferSize; i++ )
                {
                    Buffer[i] = i - 4;
                }
                wait_ms( 10 );  
                Radio.Send( Buffer, BufferSize );
            }
            else
            {
                // Send the next PONG frame
                strcpy( ( char* )Buffer, ( char* )PongMsg );
                for( i = 4; i < BufferSize; i++ )
                {
                    Buffer[i] = i - 4;
                }
                wait_ms( 10 );  
                Radio.Send( Buffer, BufferSize );
            }
            State = LOWPOWER;
            break;
        case TX_TIMEOUT:
            Radio.Rx( RX_TIMEOUT_VALUE );
            State = LOWPOWER;
            break;
        case LOWPOWER:
            break;
        default:
            State = LOWPOWER;
            break;
        }    
    }
}


void OnTxDone( void )
{
    Radio.Sleep( );
    State = TX;
    Radio.Rx( RX_TIMEOUT_VALUE );  
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    Radio.Sleep( );
    BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
    RssiValue = rssi;
    SnrValue = snr;
    State = RX;
    
    m_sxRxFrame.rxFlag=true;
    m_sxRxFrame.ptrBuf=Buffer;
    m_sxRxFrame.size=size;
    m_sxRxFrame.rssi=rssi;
    m_sxRxFrame.snr=snr;
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    State = TX_TIMEOUT;
    Radio.Rx( RX_TIMEOUT_VALUE );  
//    printf("OnTxTimeout\n\r");
//    debug_if( DEBUG_MESSAGE, "\n\r> OnTxTimeout\n\r" );
}

void OnRxTimeout( void )
{
    Radio.Sleep( );
    Buffer[ BufferSize ] = 0;
    State = RX_TIMEOUT;
    Radio.Rx( RX_TIMEOUT_VALUE );  
//    printf("OnRxTimeout\n\r");
//    debug_if( DEBUG_MESSAGE, "\n\r> OnRxTimeout\n\r" );
}

void OnRxError( void )
{
    Radio.Sleep( );
    State = RX_ERROR;
    Radio.Rx( RX_TIMEOUT_VALUE );  
//    printf("OnRxError\n\r");
//    debug_if( DEBUG_MESSAGE, "\n\r> OnRxError\n\r" );
}
/*
*/
