/****************************************************************************
* Simple UART library for devices with USART0 or UART1                      *
*                                                                           *
* author Jakub Telatnik j[dot]telatnik[at]gmail[dot]com                     *
*                                                                           *
* USAGE: See summaries below. Simple get, put and print  ascii (signed/un-  *
*        signed) integers and hex. No string support.                       *
*                                                                           *
****************************************************************************/

#include "uart.h"

#if defined (_UART0)
void uart0_init(uint16_t brate)
{
    uint16_t ubrr = (F_CPU/8UL/brate)-1;
    UBRR0L = (ubrr& 0xFF); 
    UBRR0H = (ubrr>>8);

    UCSR0A = 0x02;
    UCSR0C = 0x06;
    UCSR0B = (uint8_t)((1<<TXEN0));
}

//output the raw byte
void uart0_put(const char data_char)
{
//                while ( (UCSR0A & (1<<UDRE0))==0 );
//                UDR0 = data_char;

    while( !(UCSR0A & (1<<UDRE0)) ) ;
    UDR0 = data_char;
}

//output as ascii formatted hex, format is '0xFFFFFFFF', auto truncates empty most significant nibbles
void uart0_print_hex(uint32_t data_uint)
{
    int8_t ms_nibble;
    uint8_t nibble;
    uart0_put('0');
    uart0_put('x');
    for(ms_nibble = 28 ; ms_nibble >= 0; ms_nibble -=4)
    {
        if( (data_uint>>ms_nibble) & 0x0F) break; 
    }
    for( ; ms_nibble >= 0; ms_nibble -=4)
    {
        nibble = (uint8_t)( (data_uint>>ms_nibble) & 0x0F);
        if(nibble>0x09) nibble += 0x37;
        else nibble += 0x30;
        uart0_put(nibble); 
    }
}

//prints ascii formatted unsigned int
void uart0_print_uint(uint32_t data_uint)
{
    uint32_t ms_digit;
    uint8_t digit;
    if(data_uint==0) uart0_put('0');
    else
    {
        for(ms_digit = 1000000000; ms_digit>=1; ms_digit /= 10)
        {
            if(data_uint >= ms_digit) break;
        }
        for( ; ms_digit>=1; ms_digit /= 10)
        {
            digit = (uint8_t)(data_uint/ms_digit);
            data_uint = data_uint % ms_digit;
            uart0_put(digit + 0x30);  
        }
    }   
}

//prints ascii formatted signed int
void uart0_print_int(int32_t data_int)
{
    if(data_int < 0)
    {
        uart0_put('-');
        data_int *= (-1);
    }
    uart0_print_uint((uint32_t)(data_int));
}

uint8_t uart0_get(void)
{
    while( !(UCSR0A & (1<<RXC0)) );
    return(UDR0);
}

#endif /*defined _UART0*/

#if defined (_UART1)
void uart1_init(uint16_t brate)
{
    uint16_t ubrr = (F_CPU/8UL/brate)-1;
    UBRR1L = (ubrr& 0xFF); 
    UBRR1H = (ubrr>>8);

    UCSR1A = 0x02;
    UCSR1C = 0x06;
    UCSR1B = (uint8_t)((1<<TXEN1));
}

//output the raw byte
void uart1_put(const char data_char)
{
//                while ( (UCSR0A & (1<<UDRE0))==0 );
//                UDR0 = data_char;

    while( !(UCSR1A & (1<<UDRE1)) ) ;
    UDR1 = data_char;
}

//output as ascii formatted hex, format is '0xFFFFFFFF', auto truncates empty most significant nibbles
void uart1_print_hex(uint32_t data_uint)
{
    int8_t ms_nibble;
    uint8_t nibble;
    uart1_put('0');
    uart1_put('x');
    for(ms_nibble = 28 ; ms_nibble >= 0; ms_nibble -=4)
    {
        if( (data_uint>>ms_nibble) & 0x0F) break; 
    }
    for( ; ms_nibble >= 0; ms_nibble -=4)
    {
        nibble = (uint8_t)( (data_uint>>ms_nibble) & 0x0F);
        if(nibble>0x09) nibble += 0x37;
        else nibble += 0x30;
        uart1_put(nibble); 
    }
}

//prints ascii formatted unsigned int
void uart1_print_uint(uint32_t data_uint)
{
    uint32_t ms_digit;
    uint8_t digit;
    if(data_uint==0) uart1_put('0');
    else
    {
        for(ms_digit = 1000000000; ms_digit>=1; ms_digit /= 10)
        {
            if(data_uint >= ms_digit) break;
        }
        for( ; ms_digit>=1; ms_digit /= 10)
        {
            digit = (uint8_t)(data_uint/ms_digit);
            data_uint = data_uint % ms_digit;
            uart1_put(digit + 0x30);  
        }
    }   
}

//prints ascii formatted signed int
void uart1_print_int(int32_t data_int)
{
    if(data_int < 0)
    {
        uart1_put('-');
        data_int *= (-1);
    }
    uart1_print_uint((uint32_t)(data_int));
}

uint8_t uart1_get(void)
{
    while( !(UCSR1A & (1<<RXC1)) );
    return(UDR1);
}
#endif /*defined _UART1*/

