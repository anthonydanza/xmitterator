/****************************************************************************
* ATMEGA128RFA1 testing code - Generic broadband sensor device              *
*                                                                           *
* author Jakub Telatnik j[dot]telatnik[at]gmail[dot]com                     *
* edited/ruined by Tom Hobson                                               *
* Receive beacons, mutate the beacon message, retransmit, debug on UART0    *
****************************************************************************/
#define _UART0
#define F_CPU 16000000UL

//SETUP PARAMS 
#define _FIND_PAN   // ifdef, will look for a pan by searching for beacons
                    // else, uses values defined in PAN PARAMS 

#define THIS_CHANNEL            21    

//PAN PARAMS 
#define THIS_PAN_ID             0x5940
#define THIS_SHORT_ADDR         0x0260
#define PAN_COORD_SHORT_ADDR    0x0260
#define THIS_LONG_ADDR          0
#define THIS_BO            TRX_BEACON_ORDER_7 // ~2 seconds 
#define THIS_SO            TRX_BEACON_ORDER_5 // ~0.5 seconds

#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>

#include "uart.c"
#include "trx24.c"

// gloabal device params 

uint8_t hello_message[] = {'H','e','l','l','o',' ','P','A','N','-','C','!'};
uint8_t mutant_i = 0;

// general globals
uint8_t ADC_data = 0;


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
ISR(ADC_vect) {

ADC_data = ADCH;	//put left-adjusted 8-bit val into ADC_data

if(!(trx24PLMA_SET_TRX_STATE(TRX_STATE_PLL_ON))) err(53);	//turn PLL on to tx

ADCSRA |= 0x08;	        //reenable ADC interrupts

}

ISR(TRX24_RX_END_vect)
{
   trx24_set_rx_safe();

// print the MAC data payload and some addressing info if data frame was correctly delivered.
    uart0_put('\n'); 
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
    uart0_print_uint(TRX_FB_START(TST_RX_LENGTH));
    uart0_put('\n'); 
    uart0_put('\r');
    
    uint8_t i;
    
    for(i = 0; i<TST_RX_LENGTH; i++)
        { uart0_put(TRX_FB_START(i)); }

   trx24_clear_rx_safe();

   if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_PLL_ON))) err(53);

   if(!(trx24MCPS_DATA(hello_message, 12, TRX_FB_START(2), TRX_SEND_INTRAPAN|TRX_SEND_SRC_SHORT_ADDR|TRX_SEND_DEST_SHORT_ADDR, THIS_PAN_ID, 0x13A))) err(55) ;
   if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_RX_ON))) err(56);
}


ISR(TRX24_TX_END_vect)
{
   //check if ADC_data has been updated, transmit updated val

//   if(!(trx24PLMA_SET_TRX_STATE(TRX_STATE_PLL_ON))) err(53);

   if(!(trx24MCPS_DATA(ADC_data, 1, TRX_FB_START(2), TRX_SEND_INTRAPAN\TRX_SEND_SRC_SHORT_ADDR|TRX_SEND_DEST_SHORT_ADDR, THIS_PAN_ID, 0x13A))) err(55);

//only tx will be the beacon, set to RX_ON state after beacon is tx'd
  // if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_RX_ON))) err(10);
//do I need to turn pll off now? I want to sample ADC, tx, sample, tx etc.

}

ISR(SCNT_CMP2_vect)
{
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
    uart0_put('\n'); 
    uart0_put('\r');
   if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_TRX_OFF))) err(11);
}

ISR(SCNT_CMP3_vect)
{  
//use this timer to start listening again 

    uart0_put('\n'); 
    uart0_put('\r');
    uart0_put('R'); 
    uart0_put('X'); 
    uart0_put('_'); 
    uart0_put('O'); 
    uart0_put('N'); 
    uart0_put('\n'); 
    uart0_put('\r');

    if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_RX_ON)))  err(25);   
}

//-------------------------------------------------


int main(void)
{
    cli();
    uint32_t timer_set;    

//remember the SPI & TWI pins are shorted so make sure one set (or both) are high-Z inputs

    uart0_init(57600);

    uart0_put('\n');
    uart0_put('\r');
    uart0_put('i');
    uart0_put('n');
    uart0_put('i');
    uart0_put('t');
    uart0_put(' ');
    uart0_put('t');
    uart0_put('r');
    uart0_put('x');
    uart0_put('\n');
    uart0_put('\r');
    
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
    timer_set -= 50;    //wake up slightly early
    SCOCR3HH = (uint8_t)(timer_set >> 24);
    SCOCR3HL = (uint8_t)(timer_set >> 16);
    SCOCR3LH = (uint8_t)(timer_set >>  8);
    SCOCR3LL = (uint8_t)(timer_set & 0xFF);
    
    //this is the duration, in symbols, of the active beacon period
    timer_set = (uint32_t)(960) << (uint8_t)(THIS_SO>>8);
    SCOCR2HH = (uint8_t)(timer_set >> 24);
    SCOCR2HL = (uint8_t)(timer_set >> 16);
    SCOCR2LH = (uint8_t)(timer_set >>  8);
    SCOCR2LL = (uint8_t)(timer_set & 0xFF);

    //enable ADC, free-running auto-trigger mode
    ADCSRA |= 0x80;	//enable ADC
    ADCSRA |= 0x20;	//auto-trigger enable
    ADCSRB &= 0xF8;	//free running mode
    ADMUX |= 0x20;	//left-adjust ADC result

    //intialize the first beacon and let the rest be handled by the ISRs

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
    uart0_put('\n');
    uart0_put('\r');

    if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_RX_ON)))
        err(24);   
    sei();
    ADCSRA |= 0x40;	//start first ADC conversion
    while(1) continue;
}
