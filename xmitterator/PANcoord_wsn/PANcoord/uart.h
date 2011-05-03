#ifndef  _UARTUTILS
#define  _UARTUTILS
/****************************************************************************
* Simple UART library for devices with USART0 or UART1                      *
*                                                                           *
* author Jakub Telatnik j[dot]telatnik[at]gmail[dot]com                     *
*                                                                           *
* USAGE: See summaries below. Simple get, put and print  ascii (signed/un-  *
*        signed) integers and hex. No string support.                       *
*                                                                           *
****************************************************************************/

#if defined (__AVR_ATmega48__) || defined (__AVR_ATmega48A__) || defined (__AVR_ATmega48PA__) || defined (__AVR_ATmega48P__)
#    define _UART0
#elif defined (__AVR_ATmega88__) || defined (__AVR_ATmega88A__) || defined (__AVR_ATmega88PA__) || defined (__AVR_ATmega88P__)
#    define _UART0
#elif defined (__AVR_ATmega168__) || defined (__AVR_ATmega168A__) || defined (__AVR_ATmega168PA__) || defined (__AVR_ATmega168P__)
#    define _UART0
#elif defined (__AVR_ATmega328__) || defined (__AVR_ATmega328A__) || defined (__AVR_ATmega328PA__) || defined (__AVR_ATmega328P__)
#    define _UART0
#elif defined (__AVR_ATmega128RFA1__)
#    define _UART0
#    define _UART1
#else 
#    error "MCU TYPE IS NOT SUPPORTED BY UART UTILITY FILE, please update the uart utility file. --The Corperation"
#endif

#if defined (_UART0)
extern void uart0_init(uint16_t brate);
extern void uart0_put(const char data_char);
extern void uart0_print_hex(uint32_t data_uint);
extern void uart0_print_uint(uint32_t data_uint);
extern void uart0_print_int(int32_t data_int);
extern uint8_t uart0_get(void);
#endif

//compatability for old code with single uart defines, or for simplicity
#if defined (_SIMPLE_UART) || defined (_UART0)
//simple initialize, select baudrate, U2X is set, standard async/8-bit/no-pairity/1 stop-bit
void uart_init(uint16_t brate) {uart0_init(brate);}
//output the raw byte
void uart_put(const char data_char) {uart0_put(data_char);}
//output as ascii formatted hex, format is '0xFFFFFFFF', auto truncates empty most significant nibbles
void uart_print_hex(uint32_t data_uint) {uart0_print_hex(data_uint);}
//prints ascii formatted unsigned int
void uart_print_uint(uint32_t data_uint) {uart0_print_uint(data_uint);}
//prints ascii formatted signed int
void uart_print_int(int32_t data_int) {uart0_print_int(data_int);}
//returns char
uint8_t uart_get(void) {return uart0_get();}
#endif


#if defined (_UART1)
extern void uart1_init(uint16_t brate);
extern void uart1_put(const char data_char);
extern void uart1_print_hex(uint32_t data_uint);
extern void uart1_print_uint(uint32_t data_uint);
extern void uart1_print_int(int32_t data_int);
extern uint8_t uart1_get(void);
#endif

#endif
