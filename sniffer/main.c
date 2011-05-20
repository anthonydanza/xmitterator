/****************************************************************************
* ATMEGA128RFA1 testing code - PROMISCUOUS/SNIFFER DEVICE                   *
*                                                                           *
* author Jakub Telatnik j[dot]telatnik[at]gmail[dot]com                     *
*                                                                           *
* Listen for all, incoming packets all the time, print debug to UART0       *
****************************************************************************/
#define _UART0
#define F_CPU 16000000UL

#define THIS_PAN_ID        0x5940
#define THIS_SHORT_ADDR    0x013A
#define THIS_LONG_ADDR     0
#define THIS_CHANNEL       21    

#define THIS_BO            TRX_BEACON_ORDER_7 // ~2 seconds 
#define THIS_SO            TRX_BEACON_ORDER_5 // ~0.5 seconds


#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>

#include "uart.c"
#include "trx24.c"

uint8_t recv_val = 1;


//-------------------------------------------------

void err(uint8_t num)
{
    cli();
    uart0_put('E');
    uart0_put('R');
    uart0_put('R');
    uart0_put(' ');
    uart0_print_uint(num);
    uart0_put('\n');
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

// print the MAC data payload and some addressing info if data frame was correctly delivered.
  /*  uart0_put('\n'); 
    uart0_put('\r');
    uart0_put('<'); 
    uart0_put('<'); 
    uart0_put('<'); 
    uart0_put('R'); 
    uart0_put('X'); 
    uart0_put('\n'); 
    uart0_put('\r');
    uart0_put('L'); 
    uart0_put('Q'); 
    uart0_put('I'); 
    uart0_put(' '); 
   // uart0_print_uint(TRX_FB_START(TST_RX_LENGTH));
   // uart0_put('\n'); 
   // uart0_put('\r');*/
      uart0_put('\n'); 
    uart0_put('\r');
    uint8_t i;
 //   uint8_t recv_val_offset = TRX_FB_START(trx24MCPS_DATA_msdu());
   // uart0_print_uint(recv_val_offset);
 //   uart0_print_uint(TRX_FB_START(recv_val_offset));
      uart0_put('\n'); 
    uart0_put('\r');
    for(i = 0; i<TST_RX_LENGTH; i++)
        { uart0_print_uint(TRX_FB_START(i)); }

   if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_RX_ON)))
        err(24);  

   trx24_clear_rx_safe();
}

ISR(TRX24_RX_START_vect)
{

 /*   uart0_put('\n'); 
    uart0_put('\r');
    uart0_put('R'); 
    uart0_put('S'); 
    uart0_put('S'); 
    uart0_put(' '); 
//    uart0_print_uint((PHY_RSSI & 0x1F));
    uart0_put('\n'); 
    uart0_put('\r');
 //   uart0_put('E'); 
  //  uart0_put('D'); 
    uart0_put(' '); 
 //   uart0_print_hex(PHY_ED_LEVEL);
    uart0_put('\n'); 
    uart0_put('\r');*/
}
//-------------------------------------------------


int main(void)
{
    cli();

//remember the SPI & TWI pins are shorted so make sure one set (or both) are high-Z inputs

    uart0_init(57600);

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
    
    if(!(trx24_init(TRX_INIT_PROMISCUOUS|TRX_INIT_RECV_RES_FRAMES, THIS_CHANNEL))) err(1);

    trx24_en_autotimestamp();

    trx24_set_IRQ(TRX_IRQ_RX_END);
    trx24_set_IRQ(TRX_IRQ_RX_START);
    
    trx24_sc_enable();

    uart0_put('\n');
    uart0_put('\r');
    uart0_put('l');
    uart0_put('i');
    uart0_put('s');
    uart0_put('t');
    uart0_put('e');
    uart0_put('n');
    uart0_put('i');
    uart0_put('n');
    uart0_put('g');
    uart0_put('\r');
    uart0_put('\n');

    if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_RX_ON)))
        err(24);   
    sei();

    while(1) continue;
}
