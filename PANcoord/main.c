/****************************************************************************
* ATMEGA128RFA1 testing code - PAN COORDINATOR                              *
*                                                                           *
* author Jakub Telatnik j[dot]telatnik[at]gmail[dot]com                     *
*                                                                           *
* Send out beacons, receive data frames, print out debugging info on UART0  *
****************************************************************************/
#define _UART0
#define F_CPU 16000000UL

#define _DEBUG0
//#define _DEBUG1
#define _MUTANT

#define THIS_PAN_ID        0x5940
#define THIS_SHORT_ADDR    0x013A
#define THIS_LONG_ADDR     0
#define THIS_CHANNEL       21    

#define THIS_BO            TRX_BEACON_ORDER_8 // ~2 seconds 
#define THIS_SO            TRX_BEACON_SFRAME_ORDER_2 // ~0.5 seconds


#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>

#include "uart.c"
#include "trx24.c"

uint8_t hello_message[] = {'H','e','l','l','o',' ','W','o','r','l','d','!'};
uint8_t mutant_i = 0;

//-------------------------------------------------

void err(uint8_t num)
{
    cli();
    uart0_put('E');
    uart0_put('R');
    uart0_put('R');
    uart0_put(' ');
    uart0_print_uint(num);
    uart0_put('\r');
    uart0_put('\n');
    uart0_print_hex(trx24PLME_SET_TRX_STATE_confirm());
    uart0_put('\r');
    uart0_put('\r');

    while(1) continue;
}

//-------------------------------------------------

ISR(TRX24_RX_END_vect)
{
   trx24_set_rx_safe();

#if defined(_DEBUG0)
// print the MAC data payload and some addressing info if data frame was correctly delivered.
    uart0_put('\n'); 
    uart0_put('\r');
    uart0_put('<'); 
    uart0_put('<'); 
    uart0_put('<'); 
    uart0_put('R'); 
    uart0_put('X'); 
    uart0_put('\r');
    uart0_put('\n'); 
    uart0_put('L'); 
    uart0_put('Q'); 
    uart0_put('I'); 
    uart0_put(' '); 
    uart0_print_uint(TRX_FB_START(TST_RX_LENGTH));
    uart0_put('\r');
    uart0_put('\n'); 
    uart0_put('R'); 
    uart0_put('S'); 
    uart0_put('S'); 
    uart0_put(' '); 
    uart0_print_uint((PHY_RSSI & 0x1F));
    uart0_put('\r');
   uart0_put('\n'); 
  
    uint8_t i;
  
    for(i = 0; i<TST_RX_LENGTH; i++)
        { uart0_put(TRX_FB_START(i)); }
#endif


   trx24_clear_rx_safe();
}


ISR(TRX24_TX_END_vect)
{
//only tx will be the beacon, set to RX_ON state after beacon is tx'd
   if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_RX_ON))) err(10);
}

ISR(SCNT_CMP2_vect)
{
#if defined(_DEBUG0)
//use this timer to turn off the tranceiver at the end of the SuperFrame order
    uart0_put('\n'); 
    uart0_put('\r');
    uart0_put('Z'); 
    uart0_put('Z'); 
    uart0_put('Z'); 
    uart0_put('z'); 
    uart0_put('z'); 
    uart0_put('z'); 
    uart0_put('Z'); 
    uart0_put('Z'); 
    uart0_put('z'); 
    uart0_put('z'); 
    uart0_put('.'); 
    uart0_put('.'); 
    uart0_put('.'); 
    uart0_put('\r');
    uart0_put('\n'); 
#endif
#if defined(_MUTANT)
   if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_TRX_OFF))) err(11);
   if(PHY_RSSI & 0x40) 
       {
        if(mutant_i ==11) mutant_i = 0;
        else mutant_i++;
       }
   else 
       {
        if(mutant_i ==0) mutant_i = 11;
        else mutant_i--;
       }
   if(PHY_RSSI & 0x20) hello_message[mutant_i]++;
   else hello_message[mutant_i]--;
#endif
}

ISR(SCNT_CMP3_vect)
{  
//use this timer to call the beacon funct 

#if defined(_DEBUG0)
    uart0_put('\r');
    uart0_put('\n'); 
    uart0_put('>'); 
    uart0_put('>'); 
    uart0_put('>'); 
    uart0_put('B'); 
    uart0_put('e'); 
    uart0_put('a'); 
    uart0_put('c'); 
    uart0_put('o'); 
    uart0_put('n'); 
    uart0_put('\r');
    uart0_put('\n'); 
#endif

    if(!(trx24_pan_beacon(TRX_BEACON_SHORT_ADDR|THIS_BO|THIS_SO, hello_message,12))) err(13);
}

//-------------------------------------------------


int main(void)
{
    cli();
    uint32_t timer_set;    

//remember the SPI & TWI pins are shorted so make sure one set (or both) are high-Z inputs

    uart0_init(57600);

#if defined(_DEBUG0)
    uart0_put('\r');
    uart0_put('\n');
    uart0_put('i');
    uart0_put('n');
    uart0_put('i');
    uart0_put('t');
    uart0_put(' ');
    uart0_put('t');
    uart0_put('r');
    uart0_put('x');
    uart0_put('\r');
    uart0_put('\n');
#endif
    
    if(!(trx24_init(TRX_INIT_PAN_COORD|TRX_INIT_TX_AUTO_CRC, THIS_CHANNEL))) err(1);

    if(!(trx24_set_address(THIS_SHORT_ADDR, THIS_PAN_ID, THIS_LONG_ADDR))) err(2);

    trx24_en_autotimestamp();

    
    trx24_set_relative_compare(2);
    trx24_set_relative_compare(3);

    trx24_set_SCIRQ(TRX_SCI_CP2);
    trx24_set_SCIRQ(TRX_SCI_CP3);
    trx24_set_IRQ(TRX_IRQ_RX_END);
    trx24_set_IRQ(TRX_IRQ_TX_END);
    
    //this is the interval, in number of symbols, between beacons
    timer_set = (uint32_t)(960) << (uint8_t)(THIS_BO>>4);
 #if defined(_DEBUG1)
    uart0_put('\r');
    uart0_put('\n');
    uart0_put('>');
    uart0_print_hex(timer_set);
    uart0_put('\r');
    uart0_put('\n');
#endif
    timer_set -= 50;    //wake up slightly early
 #if defined(_DEBUG1)
    uart0_put('\r');
    uart0_put('\n');
    uart0_put('>');
    uart0_print_hex(timer_set);
    uart0_put('\r');
    uart0_put('\n');
#endif
    SCOCR3HH = (uint8_t)(timer_set >> 24);
    SCOCR3HL = (uint8_t)(timer_set >> 16);
    SCOCR3LH = (uint8_t)(timer_set >>  8);
    SCOCR3LL = (uint8_t)(timer_set & 0xFF);
    
    //this is the duration, in symbols, of the active beacon period
    timer_set = (uint32_t)(960) << (uint8_t)(THIS_SO>>8);
#if defined(_DEBUG1)
    uart0_put('\r');
    uart0_put('\n');
    uart0_put('>');
    uart0_print_hex(timer_set);
    uart0_put('\r');
    uart0_put('\n');
#endif
    SCOCR2HH = (uint8_t)(timer_set >> 24);
    SCOCR2HL = (uint8_t)(timer_set >> 16);
    SCOCR2LH = (uint8_t)(timer_set >>  8);
    SCOCR2LL = (uint8_t)(timer_set & 0xFF);

    //intialize the first beacon and let the rest be handled by the ISRs

    trx24_sc_enable();

#if defined(_DEBUG0)
    uart0_put('\r');
    uart0_put('\n');
    uart0_put('i');
    uart0_put('n');
    uart0_put('i');
    uart0_put('t');
    uart0_put(' ');
    uart0_put('b');
    uart0_put('e');
    uart0_put('a');
    uart0_put('c');
    uart0_put('o');
    uart0_put('n');
    uart0_put('\r');
    uart0_put('\n');
#endif

    if(!(trx24_pan_beacon(TRX_BEACON_SHORT_ADDR|THIS_BO|THIS_SO, hello_message,12))) err(3);
    sei();

    while(1) continue;
}
