#include "mbed.h"
#include "main.h"
#include "sx1276-hal.h"
#include "debug.h"
#include "sx1276Exe.h"

DigitalOut led(LED1);
DigitalOut RxEnable(p13);
DigitalOut TxEnable(p12);

volatile AppStates_t State = LOWPOWER;
static RadioEvents_t RadioEvents;

SX1276MB1xAS Radio( NULL );

uint16_t BufferSize = BUFFER_SIZE;
uint8_t m_sxRxBuff[BUFFER_SIZE];
uint8_t m_sxTxBuff[BUFFER_SIZE];

sxRxFrame_t m_sxRxFrame = {0,};

sx1276Exe::sx1276Exe(){
    TxEnable = 0;
    RxEnable = 1;
}
void sx1276Exe::enableSxTx(){
    TxEnable = 1;
    RxEnable = 0;
}
void disableSxTx(){
    TxEnable = 0;
    RxEnable = 1;
}
void sx1276Exe::sendLoRa(sxTxFrame_t sFrame){
    enableSxTx();
    /*
    if(sFrame.size>BUFFER_SIZE){
        printf("size is over: %d\n\r",sFrame.size);
        sFrame.size = BUFFER_SIZE;
    }
    */
    Radio.Send( (uint8_t*)sFrame.ptrBuf, sFrame.size );
//    printf("sendLoRa\n\r");
}
bool sx1276Exe::isSxRxReady(){
    return m_sxRxFrame.sxRxFlag;
}
void sx1276Exe::clearSxRxFlag(){
    m_sxRxFrame.sxRxFlag=false;
}

sxRxFrame_t* sx1276Exe::readLoRa(){
//    printf("readLoRa\n\r");
    return &m_sxRxFrame;
}

void OnTxDone( void )
{
    Radio.Sleep( );
    State = TX;
    Radio.Rx( RX_TIMEOUT_VALUE ); 
    disableSxTx();
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    Radio.Sleep( );
    memcpy( m_sxRxBuff, payload, size );
    State = RX;
    
    m_sxRxFrame.sxRxFlag=true;
    m_sxRxFrame.ptrBuf=(sxFrame_t*)m_sxRxBuff;
    m_sxRxFrame.size=size;
    m_sxRxFrame.rssi=rssi;
    m_sxRxFrame.snr=snr;
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    State = TX_TIMEOUT;
    Radio.Rx( RX_TIMEOUT_VALUE );  
    printf("\n\r OnTxTimeout \n\r"); 
}

void OnRxTimeout( void )
{
    Radio.Sleep( );
    m_sxRxBuff[ BufferSize ] = 0;
    State = RX_TIMEOUT;
    Radio.Rx( RX_TIMEOUT_VALUE );  
    printf("\n\r OnRxTimeout \n\r"); 
}

void OnRxError( void )
{
    Radio.Sleep( );
    State = RX_ERROR;
    Radio.Rx( RX_TIMEOUT_VALUE );  
    printf("\n\r OnRxError \n\r"); 
}


void sx1276Exe::testTxRx(uint32_t uiTest){
    uint8_t LoRaBuf[64];
    for(int i = 0; i<sizeof(LoRaBuf); i++) LoRaBuf[i] = i%10 + '0';
    sxTxFrame_t sTxFrame;
//    int iSize=sprintf((char*)LoRaBuf,"C:%d",uiTest);
    printf("-------------testTxRx:%d\n\r",sizeof(LoRaBuf));
    int iSize = sizeof(LoRaBuf);
    sTxFrame.ptrBuf=(sxFrame_t*)LoRaBuf; sTxFrame.size=(uint8_t)iSize; 
//    wait_ms( 20 );            
    sendLoRa(sTxFrame);
//    Radio.Rx( RX_TIMEOUT_VALUE );
}

void sx1276Exe::initSx1276(){
	m_sxRxFrame.ptrBuf = (sxFrame_t*)m_sxRxBuff;
}
void sx1276Exe::initSx1276(uint8_t ucCh){
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
        uint32_t ulCh = LoRaFreqBase + LoRaStep*ucCh;
    Radio.SetChannel( ulCh );  
        printf("LoRaChannel = %d, %d\n\r", ucCh, ulCh); 
//    Radio.SetChannel( RF_FREQUENCY );     
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
    led = 0;
        
    Radio.Rx( RX_TIMEOUT_VALUE );
//    testSx1276();
//    testTxRx();
}


/*
*/
uint8_t Buffer[BUFFER_SIZE];

const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";
#define DeTxDelay   1000

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
