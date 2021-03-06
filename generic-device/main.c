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

//ADC Params
#define BUFFER_SIZE 2
#define ADC_FS 44100

#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "uart.c"
#include "trx24.c"

// gloabal device params 

uint8_t mutant_i = 0;

// general globals
uint8_t ADC_data[BUFFER_SIZE] = {0};
uint8_t buffer_index = 0;
uint8_t ADC_bottom_bits = 0;
uint8_t fake = 0;


//-------------------------------------------------

void err(uint8_t num)
{
    cli();
    uart0_print_str("\n\rERR ",6); 
    uart0_print_uint(num);
    uart0_print_str("\n\r",2); 
    uart0_print_hex(trx24PLME_SET_TRX_STATE_confirm());
    while(1) continue;
}

//-------------------------------------------------
ISR(ADC_vect) {
//  uart0_print_str('adc interrupt',13);

    ADC_bottom_bits = ADCL;
   // ADC_data = ADCH;		//put left-adjusted 8-bit val into ADC_data

     
  //    uart0_print_str("ADC_data: ",10);
   //   uart0_print_uint(ADC_data); 
   //   uart0_print_str("\n\r",2);  
    

    if(buffer_index > BUFFER_SIZE) {
  	 // uart0_print_uint(buffer_index);	
 	 //  uart0_print_str("---------------\n\r",17);  
   	 buffer_index = 0;
   	 if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_PLL_ON))) err(53);	//turn PLL on to tx
   //   uart0_print_str("\n\r",2);  
    	uart0_print_uint(trx24MCPS_DATA(&ADC_data, 8*BUFFER_SIZE, TRX_FB_START(2), TRX_SEND_INTRAPAN|TRX_SEND_SRC_SHORT_ADDR|TRX_SEND_DEST_SHORT_ADDR, THIS_PAN_ID, 0x13A)); 
   }else {
	ADC_data[buffer_index] = ADCH;
   	buffer_index++;

   }

    //ADCSRA |= (1 << ADSC);      // Start A2D Conversions 
}

ISR(TIMER1_OVF_vect){

    TIMSK1 &=(0<<TOIE1);

    uart0_print_str("timer", 5);


    ADCSRA |= (1 << ADIE);  // Enable ADC Interrupt 
    ADCSRA |= (1 << ADSC);      // Start A2D Conversions 
    sei();
    TIMSK1 |=(1<<TOIE1);

}

ISR(TIMER1_COMP_vect) {

    TIMSK1 &= (0<<OCIE1A);

    uart0_print_str("timer", 5);


    ADCSRA |= (1 << ADIE);  // Enable ADC Interrupt 
    ADCSRA |= (1 << ADSC);      // Start A2D Conversions 

    TIMSK1 |= (1<<OCIE1A);
}


ISR(TRX24_RX_END_vect)
{
   trx24_set_rx_safe();

// print the MAC data payload and some addressing info if data frame was correctly delivered.
//    uart0_print_str("\n\r<<<RX\nLQI ",12);    
//    uart0_print_uint(TRX_FB_START(TST_RX_LENGTH));
//    uart0_print_str("\n\r",2);    
    
    uint8_t i;
    
    for(i = 0; i<TST_RX_LENGTH; i++)
        { uart0_put(TRX_FB_START(i)); }

   trx24_clear_rx_safe();

   if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_PLL_ON))) err(53);

}


ISR(TRX24_TX_END_vect)
{
   if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_PLL_ON))) err(53);
}



//-------------------------------------------------


int main(void)
{
    cli();
    uint32_t timer_set;    

//remember the SPI & TWI pins are shorted so make sure one set (or both) are high-Z inputs

    uart0_init(57600);

    uart0_print_str("\n\rINIT TRX\n\r",12); 

    if(!(trx24_init(TRX_INIT_PAN_COORD|TRX_INIT_TX_AUTO_CRC, THIS_CHANNEL))) err(1);

    if(!(trx24_set_address(THIS_SHORT_ADDR, THIS_PAN_ID, THIS_LONG_ADDR))) err(2);

    trx24_en_autotimestamp();

    
    trx24_set_relative_compare(2);
    trx24_set_relative_compare(3);

/*    trx24_set_SCIRQ(TRX_SCI_CP2);
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

    //intialize the first beacon and let the rest be handled by the ISRs

    trx24_sc_enable(); 			*/




    PRR0 &= (0 << PRADC);   //make sure power reduction disabled on A
    PRR0 &= (0 << PRPGA);   //make sure power reduction disabled on PGA
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // ADC prescaler
    ADCSRA &= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // ADC prescaler
    ADMUX &= (0 << REFS0); // Set ADC reference to AVCC
    ADMUX &= (0 << REFS1); 
    ADMUX &= 0xE0;	  //select ADC0 as input
    ADMUX |= (1 << ADLAR); // Left adjust ADC result 
    ADCSRB &= 0xF8;	//free running mode
    ADCSRA |= (1 << ADEN);  // Enable ADC 
    ADCSRA |= (1 << ADIE);  // Enable ADC Interrupt 
    //ADCSRA |= (1 << ADSC);      // Start A2D Conversions 
    //timer init
    TCCR1B|=(0<<CS02 | 0<<CS01) | (1<<CS00); 
    TCCR1B&=(0<<CS12 | 0<<CS11) | (1<<CS10); 
    TIMSK1 |=(1<<TOIE1);

    OCR1AH |= 0xFF;
    OCR1AL |= 0xFF;
    TCNT1 = 0;


    if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_PLL_ON)))
        err(24);   
    sei();

    while(1) continue;
}

