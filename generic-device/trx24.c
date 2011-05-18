/*---------- AVR 2.4GHz TRANCEIVER ---------------*/
/* j.telatnik                                2010 */
/* ---------------------------------------------- */
/* This library was written to take care of most  */
/* functionality of atmel based 2.4GHz IEEE       */
/* 802.15.4 tranceiver. Currently this library    */
/* only supports the ATMEGA128RFA1.               */
/*                                                */
/* See trx24.h for notes and usage.               */
/* ---------------------------------------------- */

#include "trx24.h"

#define _DEBUG1

//------------------------------------------------------------------
//                       GENERAL COMMANDS 
//------------------------------------------------------------------
// ---------------------------------------------

uint8_t trx24_init(uint16_t init_args, uint8_t trx_channel)
{
    while(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TRANSITION_IN_PROG) continue;
    if(!(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TRX_OFF || trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_PLL_ON || trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_SLEEP))
        return 0;

//wake the tranciever if nessecary
    if(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_SLEEP)
        { if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_TRX_OFF))) return 0; }

//assign a channel
    if((trx_channel > 10) && (trx_channel < 27)) 
        trx24PLME_SET_phyCurrentChannel(trx_channel);
    else
        return 0; //invalid channel parameter


// independent args 
    if(init_args & TRX_INIT_RX_SAFE_MODE) trx24_set_rx_safe();
    else trx24_clear_rx_safe();


    if(init_args & TRX_INIT_SLOTTED) trx24_set_slotted();
    else trx24_clear_slotted();

    if(init_args & TRX_INIT_TX_AUTO_CRC) trx24_set_auto_crc();
    else trx24_clear_auto_crc();
  
    if(init_args & TRX_INIT_RECV_RES_FRAMES) trx24_filter_res_frames();
    else trx24_hide_res_frames();

    if(init_args & TRX_INIT_AACK) trx24_enable_ACK();

// set up device types
    if(init_args & TRX_INIT_PAN_COORD) CSMA_SEED_1 |= 0x08; 
    else  CSMA_SEED_1 &= ~(0x08); 
   
    if(init_args & TRX_INIT_PROMISCUOUS)
        {
         trx24_clear_address(1,1,1); 
         XAH_CTRL_1 |= 0x02;
         trx24_disable_ACK(); 
        }
    else  XAH_CTRL_1 &= ~(0x02);

   return 1;
    
}

// ---------------------------------------------

uint8_t trx24_set_address(uint16_t short_addr, uint16_t pan_id, uint64_t long_addr)
{
    while(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TRANSITION_IN_PROG) continue;
    if(!(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TRX_OFF || trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_PLL_ON))
        return 0;

    if(short_addr)
        {
         SHORT_ADDR_0 = (uint8_t)(short_addr & 0xFF);
         SHORT_ADDR_1 = (uint8_t)((short_addr >> 8) & 0xFF);
        }

    if(pan_id)
        {
         PAN_ID_0 = (uint8_t)(pan_id & 0xFF);
         PAN_ID_1 = (uint8_t)((pan_id >> 8) & 0xFF);
        }
    if(long_addr)
        {
         IEEE_ADDR_0 = (uint8_t)(long_addr & 0xFF);
         IEEE_ADDR_1 = (uint8_t)((long_addr >> 8) & 0xFF);
         IEEE_ADDR_2 = (uint8_t)((long_addr >> 16) & 0xFF);
         IEEE_ADDR_3 = (uint8_t)((long_addr >> 24) & 0xFF);
         IEEE_ADDR_4 = (uint8_t)((long_addr >> 32) & 0xFF);
         IEEE_ADDR_5 = (uint8_t)((long_addr >> 40) & 0xFF);
         IEEE_ADDR_6 = (uint8_t)((long_addr >> 48) & 0xFF);
         IEEE_ADDR_7 = (uint8_t)((long_addr >> 56) & 0xFF);
        }

    return 1;
}

// ---------------------------------------------

uint8_t trx24_clear_address(uint8_t short_addr, uint8_t pan_id, uint8_t long_addr)
{
    while(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TRANSITION_IN_PROG) continue;
    if(!(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TRX_OFF || trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_PLL_ON))
        return 0;
    
    if(short_addr)
        {
         SHORT_ADDR_0 = 0;
         SHORT_ADDR_1 = 0;
        }

    if(pan_id)
        {
         PAN_ID_0 = 0;
         PAN_ID_1 = 0;
        }

    if(long_addr)
        {
         IEEE_ADDR_0 = 0;
         IEEE_ADDR_1 = 0;
         IEEE_ADDR_2 = 0;
         IEEE_ADDR_3 = 0;
         IEEE_ADDR_4 = 0;
         IEEE_ADDR_5 = 0;
         IEEE_ADDR_6 = 0;
         IEEE_ADDR_7 = 0;
        }

    return 1;
}

// ---------------------------------------------

uint8_t trx24_write(uint8_t data, int8_t offset)
{
//make sure the frame buffer can be wirtten to
    while(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TRANSITION_IN_PROG) continue;
    if(!(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TRX_OFF || trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_PLL_ON || trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_BUSY_TX_ARET))
        return 0;

//write the byte to the frame buffer
    if(offset >= 0)
        (TRX_FB_START(offset)) = data;
    else 
        (TRX_FB_END((offset*(-1)))) = data;
    return 1; 
}

// ---------------------------------------------

uint8_t trx24_writel(uint8_t *data, uint8_t len, int8_t offset)
{
    uint8_t i;
//make sure the frame buffer can be wirtten to
    while(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TRANSITION_IN_PROG) continue;
    if(!(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TRX_OFF || trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_PLL_ON || trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_BUSY_TX_ARET))
        return 0;

//write the bytes to the frame buffer
    if(offset >= 0)
        {
         if((offset + len)>127) return 0;
         for( i = 0; i < len; i ++)
             {TRX_FB_START(offset + i)  = *(data + i);}
        }
    else 
        {
         if( (offset + len) > 0 ) return 0;
         offset *= -1;
         for( i = 0; i < len; i ++)
             {TRX_FB_END(offset - i)  = *(data + i);}
        }
    return 1; 
}

//------------------------------------------------------------------
//                        DEVICE COMMANDS 
//------------------------------------------------------------------


uint8_t trx24_scan_beacon(uint32_t timeout) 
{        
// check for illegal TRX_STATE
    while(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TRANSITION_IN_PROG) continue;
    if(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_BUSY_RX || trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_BUSY_TX || trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_BUSY_RX_AACK || trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_BUSY_TX_ARET) 
        return 0; 

// turn off radio inturupts for now, 
    uint8_t save_IRQ_MASK = IRQ_MASK;
    uint8_t save_SCIRQM = SCIRQM;
    SCIRQM = 0; 
    IRQ_MASK = 0;
    

//setup the listen timeout, use SCOC unit 1
    trx24_beacon_timestamp();
    if(!timeout) timeout += 3720000UL; //defualt 1 min timeout 

    SCOCR1HH = (uint8_t)((timeout >> 24) & 0xFF);
    SCOCR1HL = (uint8_t)((timeout >> 16) & 0xFF);
    SCOCR1LH = (uint8_t)((timeout >> 8) & 0xFF);
    SCOCR1LL = (uint8_t)(timeout & 0xFF);
// dont use IRQ vectors for now, see if we can just monitor the flags
    if(SCIRQS & TRX_SCI_CP1) SCIRQS |= (uint8_t)(TRX_SCI_CP1);

//turn on the RX and listen for a PAN beacon
    if(trx24PLME_SET_TRX_STATE_confirm() != TRX_STATE_RX_ON)
        if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_RX_ON))) 
            {
             SCIRQM   = save_SCIRQM;
             IRQ_MASK = save_IRQ_MASK;
             trx24PLME_SET_TRX_STATE(TRX_STATE_PLL_ON);
             return 0;
            }
    
    while(!(SCIRQS & TRX_SCI_CP1))
        {
          if(IRQ_STATUS & TRX_IRQ_RX_END)
            {
             if(trx24_CRC_valid() && (TRXFBST & 0xE0) == 0x00)
                {
                 //stop listening and return the saved frame buffer and saves
                 SCIRQM   = save_SCIRQM;
                 IRQ_MASK = save_IRQ_MASK;
                 trx24PLME_SET_TRX_STATE(TRX_STATE_PLL_ON);
                 return 1;
                }
            }
         
        }

//return a 0 if we didn't catch a beacon
    SCIRQM   = save_SCIRQM;
    IRQ_MASK = save_IRQ_MASK;
    trx24PLME_SET_TRX_STATE(TRX_STATE_PLL_ON);
    return 0;

}
//------------------------------------------------------------------
//                    PAN COORDINATOR COMMANDS
//------------------------------------------------------------------

uint8_t trx24_pan_beacon(uint16_t beacon_args, uint8_t *beacon_payload, uint8_t beacon_payload_length)
{

static uint8_t sequence_num;
uint32_t timer_compare;
uint8_t fb_pos = 1;
uint8_t beacon_orders; //upper 4b is beacon order, lower is superframe order

//check for allowed TRX_STATE
    while(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TRANSITION_IN_PROG) continue;
    if(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_BUSY_RX || trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_BUSY_TX || trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_BUSY_RX_AACK || trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_BUSY_TX_ARET)
        return 0;

    beacon_orders = ((beacon_args & 0xF0) | ((beacon_args >> 8) & 0x0F));

    if((beacon_orders & 0x0F) > ((beacon_orders >> 4) & 0x0F))
        return 0; //incorrect beacon/superframe order settings

//set state to PLL_ON
    if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_PLL_ON)))
         return 0;


//write the MHR feild for the beacon
    trx24_write(0x00, fb_pos++);  // frame control feild
    if(beacon_args & TRX_BEACON_LONG_ADDR)
        trx24_write(0x30, fb_pos++);
    else
        trx24_write(0x20, fb_pos++);

    trx24_write(++sequence_num, fb_pos++); 

    trx24_write(PAN_ID_1, fb_pos++); //addressing feilds(source only)
    trx24_write(PAN_ID_0, fb_pos++);

    if(beacon_args & TRX_BEACON_LONG_ADDR)
        {
         trx24_write(IEEE_ADDR_7, fb_pos++);
         trx24_write(IEEE_ADDR_6, fb_pos++);
         trx24_write(IEEE_ADDR_5, fb_pos++);
         trx24_write(IEEE_ADDR_4, fb_pos++);
         trx24_write(IEEE_ADDR_3, fb_pos++);
         trx24_write(IEEE_ADDR_2, fb_pos++);
         trx24_write(IEEE_ADDR_1, fb_pos++);
         trx24_write(IEEE_ADDR_0, fb_pos++);
        }
    else
        {
         trx24_write(SHORT_ADDR_1, fb_pos++);
         trx24_write(SHORT_ADDR_0, fb_pos++);
        }
    trx24_write(beacon_orders, fb_pos++);
    trx24_write(0xF3, fb_pos++); // lower byte of the superframe specification
    trx24_write(0x00, fb_pos++); //GTS Spec    
    trx24_write(0x00, fb_pos++); //pending addresses spec

    if(beacon_payload_length < 33)
        {
         trx24_writel(beacon_payload, beacon_payload_length, fb_pos);     
         fb_pos += beacon_payload_length;
        }
    else
        {
         trx24_writel(beacon_payload, 32, fb_pos);     
         fb_pos += 32;
        }
    //write the PHY header to the first byte of the frame buffer
    trx24_write((fb_pos+1), 0); 

// set up timing for the beacon according to beacon order args
    //send right away if last timestamp is zero (init condition)
    if((SCBTSRLL != 0 || SCBTSRHL != 0 || SCBTSRLH != 0 || SCBTSRHH != 0) && ((beacon_orders & 0xF0) != 0xF0))
        {
         timer_compare = (SCBTSRLL)|((uint32_t)(SCBTSRHH) << 24)|((uint32_t)(SCBTSRHL) << 16)|(uint32_t)(SCBTSRLH << 8);
         timer_compare += ((uint32_t)0x3C0 << ((beacon_orders >> 4) & 0x0F));
         while(SCSR & 0x01)  ; //make sure the symbol counter isnt busy
         if(timer_compare < ((SCCNTLL)|((uint32_t)(SCCNTHH) << 24)|((uint32_t)(SCCNTHL) << 16)|(SCCNTLH << 8)) )
             ;
         else
             {
              trx24_set_relative_compare(1); //use SCOC unit 1
             // if(SCIRQS & 0x01) SCIRQS |= 0x01; //clear any pending flags
              timer_compare = ((uint32_t)0x3C0 << ((beacon_orders >> 4) & 0x0F)); 
              SCOCR1HH = (uint8_t)(timer_compare >> 24);
              SCOCR1HL = (uint8_t)(timer_compare >> 16);
              SCOCR1LH = (uint8_t)(timer_compare >> 8);
              SCOCR1LL = (uint8_t)(timer_compare);
             //and now we play the wating game
              while(!(SCIRQS & 0x01)) ; 
              SCIRQS |= 0x01;
             }
        } 

//and we are ready to send
    if(trx24PLME_SET_TRX_STATE(TRX_STATE_TX_START))
        trx24_beacon_timestamp();
    else 
        {
         sequence_num--;      
         return 0;
        }
    
//return 
    if(sequence_num == 0xFF)
        {
         sequence_num = 0x00;
         return 0xFF;
        }
    else
        return sequence_num;
}


/*________________________________________________________________
 |                                                                |
 |                     PHY Service                                |
 |________________________________________________________________*/

//  6.2.1 PHY Data Service
//================================

//--- 6.2.1.1 PD-DATA.request
uint8_t trx24PD_DATA(uint8_t psduLength, uint8_t *psdu)
{
    while(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TRANSITION_IN_PROG) continue;
    if(!(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_PLL_ON || trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TX_ARET_ON ))
        return 0;
    
    if(psduLength > TRX_aMaxPHYPacketSize) return 0;

    if(!(trx24_writel(psdu, psduLength, 1))) return 0;
    if(!(trx24_write(psduLength+1, 0)))      return 0;

    if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_TX_START))) return 0;  

    return 1;
}

//  6.2.2 PHY Management Service
//================================

//--- 6.2.2.7 PLME-SET-TRX-STATE.request
uint8_t trx24PLME_SET_TRX_STATE(uint8_t state)
{
    while(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TRANSITION_IN_PROG) continue;
    
// INIT STATUS == SLEEP
    if(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_SLEEP)
    {
     if (state == TRX_STATE_SLEEP)
         return 1;
     else
        {
         TRXPR &= ~(0x02); //wake the tranceiver
         while(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TRANSITION_IN_PROG) continue;
         while(trx24PLME_SET_TRX_STATE_confirm() != TRX_STATE_TRX_OFF) ; 
        }
    } 
    
// INIT STATUS == TRX_OFF
    if(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TRX_OFF)
    {
     if (state == TRX_STATE_TRX_OFF || state == TRX_STATE_FORCE_TRX_OFF)
         return 1;
     else if(state == TRX_STATE_SLEEP)
        {
         TRXPR |= 0x02; //toggle the sleep pin
         while(trx24PLME_SET_TRX_STATE_confirm() != state) ; 
         if(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_SLEEP) return 1;
         else return 0;
        }
     //allowed state transitions go here
     else if(state == TRX_STATE_PLL_ON || state == TRX_STATE_RX_ON || state == TRX_STATE_TX_ARET_ON || state == TRX_STATE_RX_AACK_ON || state == TRX_STATE_FORCE_PLL_ON )
        {
         TRX_STATE = state;
         if(state == TRX_STATE_FORCE_PLL_ON) state = TRX_STATE_PLL_ON;
         while(trx24PLME_SET_TRX_STATE_confirm() != state) ; 
         if(trx24PLME_SET_TRX_STATE_confirm() == state) return 1;
         else return 0;
        }
     else return 0;
    }

// INIT STATUS == PLL_ON
    if(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_PLL_ON)
    {
     if (state == TRX_STATE_PLL_ON  || state == TRX_STATE_FORCE_PLL_ON )
         return 1;
     else if(state == TRX_STATE_TRX_OFF || state == TRX_STATE_FORCE_TRX_OFF || state == TRX_STATE_RX_ON || state == TRX_STATE_TX_ARET_ON || state == TRX_STATE_RX_AACK_ON || state == TRX_STATE_TX_START)
         {
          TRX_STATE = state;
          if(state == TRX_STATE_FORCE_TRX_OFF) state = TRX_STATE_TRX_OFF; 
          while(trx24PLME_SET_TRX_STATE_confirm() != state) ; 
          if(trx24PLME_SET_TRX_STATE_confirm() == state) return 1;
          else return 0;
         }
     else return 0;
    }

// INIT STATUS == RX_ON
    if(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_RX_ON)
    {
     if (state == TRX_STATE_RX_ON)
         return 1;
     //allowed transitions
     else if(state == TRX_STATE_TRX_OFF || state == TRX_STATE_FORCE_TRX_OFF || state == TRX_STATE_PLL_ON || state == TRX_STATE_FORCE_PLL_ON )
         {
          TRX_STATE = state;
          if(state == TRX_STATE_FORCE_TRX_OFF) state = TRX_STATE_TRX_OFF; 
          else if(state == TRX_STATE_FORCE_PLL_ON) state = TRX_STATE_PLL_ON;
          while(trx24PLME_SET_TRX_STATE_confirm() != state) ; 
          if(trx24PLME_SET_TRX_STATE_confirm() == state) return 1;
          else return 0;
         }      
     //initial STATE was illegal
     else return 0; 
    }

// INIT STATUS == RX_AACK_ON
    if(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_RX_AACK_ON)
    {
     if (state == TRX_STATE_RX_AACK_ON)
         return 1;
     //allowed transitions
     else if(state == TRX_STATE_TRX_OFF || state == TRX_STATE_FORCE_TRX_OFF || state == TRX_STATE_PLL_ON || state == TRX_STATE_FORCE_PLL_ON )
         {
          TRX_STATE = state;
          if(state == TRX_STATE_FORCE_TRX_OFF) state = TRX_STATE_TRX_OFF; 
          else if(state == TRX_STATE_FORCE_PLL_ON) state = TRX_STATE_PLL_ON;
          while(trx24PLME_SET_TRX_STATE_confirm() != state) ; 
          if(trx24PLME_SET_TRX_STATE_confirm() == state) return 1;
          else return 0;
         }      
     //initial STATE was illegal
     else return 0; 
    }

// INIT STATUS == TX_ARET_ON
    if(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TX_ARET_ON)
    {
     if (state == TRX_STATE_TX_ARET_ON )
         return 1;
     else if(state == TRX_STATE_TRX_OFF || state == TRX_STATE_FORCE_TRX_OFF || state == TRX_STATE_PLL_ON || state == TRX_STATE_FORCE_PLL_ON || state == TRX_STATE_TX_START)
         {
          TRX_STATE = state;
          if(state == TRX_STATE_FORCE_TRX_OFF) state = TRX_STATE_TRX_OFF; 
          else if(state == TRX_STATE_FORCE_PLL_ON) state = TRX_STATE_PLL_ON;
          else if(state == TRX_STATE_TX_START) state = TRX_STATE_BUSY_TX_ARET;
          while(trx24PLME_SET_TRX_STATE_confirm() != state) ; 
          if(trx24PLME_SET_TRX_STATE_confirm() == state) return 1;
          else return 0;
         }
     else return 0;
    }

// INIT STATUS == BUSY_TX || BUSY_TX_ARET || BUSY_RX || BUSY_RX_AACK
    if((trx24PLME_SET_TRX_STATE_confirm() & 0x0F) == TRX_STATE_BUSY_TX || (trx24PLME_SET_TRX_STATE_confirm() & 0x0F) == TRX_STATE_BUSY_RX)
    {
     if(state == TRX_STATE_FORCE_TRX_OFF || state == TRX_STATE_FORCE_PLL_ON)
         {
          TRX_STATE = state;
          if(state == TRX_STATE_FORCE_TRX_OFF) state = TRX_STATE_TRX_OFF; 
          else if(state == TRX_STATE_FORCE_PLL_ON) state = TRX_STATE_PLL_ON;
          while(trx24PLME_SET_TRX_STATE_confirm() != state) ; 
          if(trx24PLME_SET_TRX_STATE_confirm() == state) return 1;
          else return 0;
         }
     else if(state == TRX_STATE_TRX_OFF || state == TRX_STATE_PLL_ON )
         {
          TRX_STATE = state;
          return 1;
         }
     else if(((trx24PLME_SET_TRX_STATE_confirm() & 0x0F) == TRX_STATE_BUSY_TX) && (state == TRX_STATE_RX_ON || state == TRX_STATE_RX_AACK_ON))
         {
          TRX_STATE = state;
          return 1;
         }
     else return 0;
    }
    return 0;
}

/*________________________________________________________________
 |                                                                |
 |                    MAC Sublayer Specification                  |
 |________________________________________________________________*/

//  7.1.1 MAC Data Service
//================================

//--- 7.1.1.1 MCPS-DATA.request
uint8_t trx24MCPS_DATA(uint8_t *msdu, uint8_t msduLength, uint8_t sequence_num, uint8_t tx_args, uint16_t DstPANId, uint64_t DstAddr)
{

    while(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TRANSITION_IN_PROG) continue;
    if(!(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TRX_OFF || trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TX_ARET_ON || trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_PLL_ON))
        return 0;

//start up the PLL
    if(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TRX_OFF)
        {	
         if(tx_args & TRX_SEND_TX_ARET_ON)
             { if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_TX_ARET_ON))) return 0;}
         else
             { if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_PLL_ON))) return 0;}
        }
    else if(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_PLL_ON && (tx_args & TRX_SEND_TX_ARET_ON))
        { if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_TX_ARET_ON))) return 0;}
    else if(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TX_ARET_ON && !(tx_args & TRX_SEND_TX_ARET_ON))
        { if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_PLL_ON))) return 0;}

   uint8_t fb_pos = 1; 
   uint8_t buffer_byte;


    if(msduLength < 8) return 0; //payload too small

//setup the Frame control feild
    buffer_byte = ((0x20) | (tx_args & 0x0E));
    trx24_write(buffer_byte, fb_pos++);
    buffer_byte =  0x00;

    if(tx_args & TRX_SEND_SRC_LONG_ADDR) buffer_byte |= 0x20;
    else buffer_byte |= 0x30;

    if(tx_args & TRX_SEND_DEST_LONG_ADDR) buffer_byte |= 0x02;
    else buffer_byte |= 0x03;
    trx24_write(buffer_byte, fb_pos++);
//setup the adressing feilds 
    if(tx_args & TRX_SEND_INTRAPAN) //destination PAN-ID
        {
         trx24_write(PAN_ID_1, fb_pos++); 
         trx24_write(PAN_ID_0, fb_pos++); 
        }
    else
        {
         trx24_write((uint8_t)(DstPANId>>8), fb_pos++); 
         trx24_write((uint8_t)(DstPANId & 0xFF), fb_pos++); 
        }
    if(tx_args & TRX_SEND_DEST_LONG_ADDR) //destination address
        {
         trx24_write((uint8_t)(DstAddr>>8), fb_pos++); 
         trx24_write((uint8_t)(DstAddr & 0xFF), fb_pos++); 
        }
    else
        {
         trx24_write((uint8_t)(DstAddr>> 56), fb_pos++); 
         trx24_write((uint8_t)(DstAddr>> 48), fb_pos++); 
         trx24_write((uint8_t)(DstAddr>> 40), fb_pos++); 
         trx24_write((uint8_t)(DstAddr>> 32), fb_pos++); 
         trx24_write((uint8_t)(DstAddr>> 24), fb_pos++); 
         trx24_write((uint8_t)(DstAddr>> 16), fb_pos++); 
         trx24_write((uint8_t)(DstAddr>>  8), fb_pos++); 
         trx24_write((uint8_t)(DstAddr & 0xFF), fb_pos++); 
        }
    if(!(tx_args & TRX_SEND_INTRAPAN)) //source PAN-ID
        {
         trx24_write(PAN_ID_1, fb_pos++); 
         trx24_write(PAN_ID_0, fb_pos++); 
        }
    if(tx_args & TRX_SEND_SRC_LONG_ADDR) //Source address
        {
         trx24_write((uint8_t)(SHORT_ADDR_1), fb_pos++); 
         trx24_write((uint8_t)(SHORT_ADDR_0), fb_pos++); 
        }
    else
        {
         trx24_write((uint8_t)(IEEE_ADDR_7), fb_pos++); 
         trx24_write((uint8_t)(IEEE_ADDR_6), fb_pos++); 
         trx24_write((uint8_t)(IEEE_ADDR_5), fb_pos++); 
         trx24_write((uint8_t)(IEEE_ADDR_4), fb_pos++); 
         trx24_write((uint8_t)(IEEE_ADDR_3), fb_pos++); 
         trx24_write((uint8_t)(IEEE_ADDR_2), fb_pos++); 
         trx24_write((uint8_t)(IEEE_ADDR_1), fb_pos++); 
         trx24_write((uint8_t)(IEEE_ADDR_0), fb_pos++); 
        }
    //AUX Security header would go here
    //write MAC payload
    if(msduLength > (128 - fb_pos -2))  //check if the data payload is too long
        return 6;
    trx24_writel(msdu, msduLength, fb_pos);
    fb_pos += (msduLength-1); 
    //go back and write the PHR (frame length)
    trx24_write(fb_pos, 0); 

    //and send away
    if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_TX_START))) return 0;  

    return 1; // all went well?
}

//--- 7.1.1.1 MCPS-DATA.confirm
uint32_t trx24MCPS_DATA_Timestamp(void)
{
    uint32_t timestamp;
    timestamp = SCTSRLL;
    timestamp |= (uint32_t)SCTSRLH << 8; 
    timestamp |= (uint32_t)SCTSRHL << 16; 
    timestamp |= (uint32_t)SCTSRHH << 24; 
    return timestamp;
}         

//--- 7.1.1.3 MCPS-DATA.indication
uint16_t trx24MCPS_DATA_SrcPANId(void)      
{
    uint16_t SrcPANId;
    if((TRX_FB_START(0) & 0x02) || trx24MCPS_DATA_DstAddrMode() == TRX_ADDR_MODE_NONE)
        {
         SrcPANId  = TRX_FB_START(4);
         SrcPANId |= TRX_FB_START(3) << 8;
         return SrcPANId;
        }
    else if(trx24MCPS_DATA_DstAddrMode() == TRX_ADDR_MODE_LONG)
        {
         SrcPANId  = TRX_FB_START(14);
         SrcPANId |= TRX_FB_START(13) << 8;
         return SrcPANId;
        }
    else if(trx24MCPS_DATA_DstAddrMode() == TRX_ADDR_MODE_SHORT)
        {
         SrcPANId  = TRX_FB_START(8);
         SrcPANId |= TRX_FB_START(7) << 8;
         return SrcPANId;
        }
    return 0; //something went wrong
}
uint64_t trx24MCPS_DATA_SrcAddr(void)      
{
    uint64_t SrcAddr = 0;
    uint8_t offset;
    
    if(trx24MCPS_DATA_SrcAddrMode() == TRX_ADDR_MODE_NONE) return 0;
    else if(trx24MCPS_DATA_DstAddrMode() == TRX_ADDR_MODE_SHORT) offset = 7;
    else offset = 13;

    if(!(TRX_FB_START(0) & 0x02)) offset += 2; //no address feild compression

    if(trx24MCPS_DATA_SrcAddrMode() == TRX_ADDR_MODE_SHORT)
        {
         SrcAddr  = TRX_FB_START(offset+1);
         SrcAddr |= (uint16_t)TRX_FB_START(offset) << 8;
        }
    else
        {
         SrcAddr  = TRX_FB_START(offset+7);
         SrcAddr |= (uint32_t)TRX_FB_START(offset+6) << 8;
         SrcAddr |= (uint32_t)TRX_FB_START(offset+5) << 16;
         SrcAddr |= (uint32_t)TRX_FB_START(offset+4) << 24;
         SrcAddr |= (uint64_t)TRX_FB_START(offset+3) << 32;
         SrcAddr |= (uint64_t)TRX_FB_START(offset+2) << 40;
         SrcAddr |= (uint64_t)TRX_FB_START(offset+1) << 48;
         SrcAddr |= (uint64_t)TRX_FB_START(offset)   << 56;
        }
    return SrcAddr;
}
uint16_t trx24MCPS_DATA_DstPANId(void)      
{
    uint16_t DstPANId;
    if(trx24MCPS_DATA_DstAddrMode() == TRX_ADDR_MODE_NONE)
        return 0;
    else
        {
         DstPANId  = TRX_FB_START(4);
         DstPANId |= TRX_FB_START(3) << 8;
         return DstPANId;
        }
    return 0; //something went wrong
}
uint64_t trx24MCPS_DATA_DstAddr(void)      
{
    uint64_t DstAddr = 0;

    if(trx24MCPS_DATA_DstAddrMode() == TRX_ADDR_MODE_NONE) return 0;
    if(trx24MCPS_DATA_DstAddrMode() == TRX_ADDR_MODE_SHORT)
        {
         DstAddr  = TRX_FB_START(6);
         DstAddr |= (uint16_t)TRX_FB_START(5) << 8;
        }
    else
        {
         DstAddr  = TRX_FB_START(12);
         DstAddr |= (uint32_t)TRX_FB_START(11) << 8;
         DstAddr |= (uint32_t)TRX_FB_START(10) << 16;
         DstAddr |= (uint32_t)TRX_FB_START(8)  << 24;
         DstAddr |= (uint64_t)TRX_FB_START(8)  << 32;
         DstAddr |= (uint64_t)TRX_FB_START(7)  << 40;
         DstAddr |= (uint64_t)TRX_FB_START(6)  << 48;
         DstAddr |= (uint64_t)TRX_FB_START(5)  << 56;
        }
    return DstAddr;

}
uint8_t  trx24MCPS_DATA_msduLength(void)
{
    uint8_t msduLength = TST_RX_LENGTH;
    uint8_t security_header_offset =3;
    msduLength -= 5; //less the FCF, FCS, and Sequence number
    if(trx24MCPS_DATA_DstAddrMode() == TRX_ADDR_MODE_LONG)
        {
         msduLength -= 10;
         security_header_offset += 10;
        }
    else if(trx24MCPS_DATA_DstAddrMode() == TRX_ADDR_MODE_SHORT)
        {
         msduLength -= 4;
         security_header_offset += 4;
        }
    if(!(TRX_FB_START(0) & 0x02)) 
        {
         msduLength -= 2;
         security_header_offset += 2;
        }
    if(trx24MCPS_DATA_SrcAddrMode() == TRX_ADDR_MODE_LONG)
        {
         msduLength -= 8;
         security_header_offset += 8;
        }
    else if(trx24MCPS_DATA_SrcAddrMode() == TRX_ADDR_MODE_SHORT)
        {
         msduLength -= 2;
         security_header_offset += 2;
        }
    if(TRX_FB_START(0) & 0x10) 
        {
         msduLength -= 5;
         if((TRX_FB_START(security_header_offset) & 0x18) == 0x08)
             msduLength -= 1;
         else if((TRX_FB_START(security_header_offset) & 0x18) == 0x10)
             msduLength -= 5;
         else if((TRX_FB_START(security_header_offset) & 0x18) == 0x18)
             msduLength -= 9;
        }
    return msduLength;
}       
uint8_t  trx24MCPS_DATA_msdu(void)        
{
    if(TST_RX_LENGTH < 6) return 0; //frame is an ACK, or invalid
    uint8_t msdu_offset = 3; 
    uint8_t security_header_offset; 
    if(trx24MCPS_DATA_DstAddrMode() == TRX_ADDR_MODE_LONG)
        msdu_offset += 10;
    else if(trx24MCPS_DATA_DstAddrMode() == TRX_ADDR_MODE_SHORT)
        msdu_offset += 4;
    if(!(TRX_FB_START(0) & 0x02)) 
        msdu_offset += 2;
    if(trx24MCPS_DATA_SrcAddrMode() == TRX_ADDR_MODE_LONG)
        msdu_offset += 8;
    else if(trx24MCPS_DATA_SrcAddrMode() == TRX_ADDR_MODE_SHORT)
        msdu_offset += 2;
    if(TRX_FB_START(0) & 0x10) 
        {
         security_header_offset = msdu_offset;
         msdu_offset += 5;
         if((TRX_FB_START(security_header_offset) & 0x18) == 0x08)
             msdu_offset += 1;
         else if((TRX_FB_START(security_header_offset) & 0x18) == 0x10)
             msdu_offset += 5;
         else if((TRX_FB_START(security_header_offset) & 0x18) == 0x18)
             msdu_offset += 9;
        }
    return msdu_offset;
}

//  7.1.3 Association Primatives
//================================

//--- 7.1.3.1 MLME-ASSOCIATE.request
uint8_t trx24MLME_ASSOCIATE_request(uint8_t assoc_args, uint8_t LogicalChannel, uint16_t CoordPANId, uint64_t CoordAddress)
{
    while(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TRANSITION_IN_PROG) continue;
    if(!(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TX_ARET_ON || trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_PLL_ON))
        return 0;

    if((LogicalChannel > 10) && (LogicalChannel < 27)) 
        trx24PLME_SET_phyCurrentChannel(LogicalChannel);
    else if(LogicalChannel) return 0; //invalid channel
    PAN_ID_1 = (uint8_t)(CoordPANId >>8);
    PAN_ID_0 = (uint8_t)(CoordPANId & 0xFF);

    uint8_t assoc_psdu[27];
    uint8_t psdu_index = 0;

    assoc_psdu[psdu_index++] = TRX_FCF_TYPE_MAC_CMD|TRX_FCF_ACK_REQ;
    assoc_psdu[psdu_index++] = (((assoc_args & TRX_MLME_SHORT_ADDR)|0x02)<<4)|0x03;
    assoc_psdu[psdu_index++] = 0x01;  //sequence num, needs to be something else
   //addressing feilds 
    assoc_psdu[psdu_index++] = (uint8_t)CoordPANId >> 8;
    assoc_psdu[psdu_index++] = (uint8_t)CoordPANId & 0xFF;
    if(assoc_args & TRX_MLME_ADDR_MODE_LONG)
        {
         assoc_psdu[psdu_index++] = (uint8_t)(CoordAddress >> 56);
         assoc_psdu[psdu_index++] = (uint8_t)(CoordAddress >> 48);
         assoc_psdu[psdu_index++] = (uint8_t)(CoordAddress >> 40);
         assoc_psdu[psdu_index++] = (uint8_t)(CoordAddress >> 32);
         assoc_psdu[psdu_index++] = (uint8_t)(CoordAddress >> 24);
         assoc_psdu[psdu_index++] = (uint8_t)(CoordAddress >> 16);
         assoc_psdu[psdu_index++] = (uint8_t)(CoordAddress >>  8);
         assoc_psdu[psdu_index++] = (uint8_t)(CoordAddress & 0xFF);
        }
    else
        {
         assoc_psdu[psdu_index++] = (uint8_t)(CoordAddress >>  8);
         assoc_psdu[psdu_index++] = (uint8_t)(CoordAddress);
        }
    assoc_psdu[psdu_index++] = PAN_ID_1;
    assoc_psdu[psdu_index++] = PAN_ID_0;
    assoc_psdu[psdu_index++] = IEEE_ADDR_7;
    assoc_psdu[psdu_index++] = IEEE_ADDR_6;
    assoc_psdu[psdu_index++] = IEEE_ADDR_5;
    assoc_psdu[psdu_index++] = IEEE_ADDR_4;
    assoc_psdu[psdu_index++] = IEEE_ADDR_3;
    assoc_psdu[psdu_index++] = IEEE_ADDR_2;
    assoc_psdu[psdu_index++] = IEEE_ADDR_1;
    assoc_psdu[psdu_index++] = IEEE_ADDR_0;
    assoc_psdu[psdu_index++] = TRX_CMD_FRAME_ID_ASSOC_REQ;
    assoc_psdu[psdu_index]  = 0x00;
    if(assoc_args & TRX_MLME_CI_ALT_PAN)
        assoc_psdu[psdu_index] |= 0x80;
    if(assoc_args & TRX_MLME_CI_FFD)
        assoc_psdu[psdu_index] |= 0x40;
    if(assoc_args & TRX_MLME_CI_MAINS_PWR)
        assoc_psdu[psdu_index] |= 0x20;
    if(assoc_args & TRX_MLME_CI_RX_ON_IDLE)
        assoc_psdu[psdu_index] |= 0x10;
    if(assoc_args & TRX_MLME_CI_ALLOC_ADDR)
        assoc_psdu[psdu_index] |= 0x01;
    psdu_index++;
    assoc_psdu[psdu_index++] = 0x00; 
    assoc_psdu[psdu_index++] = 0x00; 
    //send the Assoc Request written to assoc_psdu; make sure the auto-FCS is on
    uint8_t temp_irq_mask = IRQ_MASK; //disable this for now
    IRQ_MASK = 0x00;    //and also clear any lingering IRQs 
    if(IRQ_STATUS & TRX_IRQ_TX_END) IRQ_STATUS |= (uint8_t)(TRX_IRQ_TX_END);
    if(IRQ_STATUS & TRX_IRQ_RX_END) IRQ_STATUS |= (uint8_t)(TRX_IRQ_RX_END);
    if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_TX_ARET_ON))) return 0;
    if(!(trx24PD_DATA(psdu_index, assoc_psdu))) return 0;
    while(!(IRQ_STATUS & TX_END)) ; // wait for ACK for the assoc req
    if((TRX_STATE & 0x70) != 0x00)  return 0;
 //set up the wait (= macResponseWaitTime = 4) before sending the data request command
    if(SCIRQS & TRX_SCI_CP1) SCIRQS |= (uint8_t)(TRX_SCI_CP1);
    SCCR0  &= ~(0x01);
    SCIRQM &= ~(0x01);
    SCOCR1HH = SCTSRHH + (uint8_t)(TRX_aBaseSuperFrameDuration*4 >> 24);
    SCOCR1HL = SCTSRHL + (uint8_t)(TRX_aBaseSuperFrameDuration*4 >> 16);
    SCOCR1LH = SCTSRLH + (uint8_t)(TRX_aBaseSuperFrameDuration*4 >>  8);
    SCOCR1LL = SCTSRLL + (uint8_t)(TRX_aBaseSuperFrameDuration*4 & 0xFF);
   //set up the data reqest frame 
    psdu_index -= 5;
    assoc_psdu[psdu_index++] = TRX_CMD_FRAME_ID_DATA_REQ; 
    assoc_psdu[psdu_index++] = 0x00; 
    assoc_psdu[psdu_index++] = 0x00; 
    while(!(SCIRQS & TRX_SCI_CP1)) ; //wait out the delay and send 
    IRQ_STATUS |= (uint8_t)(TRX_IRQ_RX_END);
    if(!(trx24PD_DATA(psdu_index, assoc_psdu))) return 0;
    while(!(IRQ_STATUS & TX_END)) ; // wait for an ACK with frame pending
    if((TRX_STATE & 0x70) != 0x10)  return 0;
   //got the aack, association response incoming
    if(IRQ_STATUS & TRX_IRQ_RX_END) IRQ_STATUS |= (uint8_t)(TRX_IRQ_RX_END);
    if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_PLL_ON))) return 0;
    if(!(trx24PLME_SET_TRX_STATE(TRX_STATE_RX_AACK_ON))) return 0;
    while(!(IRQ_STATUS & RX_END)) ; // wait for an ACK with frame pending
    if((TRX_STATE & 0x70) != 0x00)  return 0;
 

   return 0; // THIS IS TEMP FOR COMPILING
}

//--- 7.1.3.2 MLME-ASSOCIATE.indication
uint8_t trx24MLME_ASSOCIATE_indication(void)
{   
    //depending on implementation, this first check may be redundant 
    if((TRX_FB_START(0) & 0xE0) != TRX_FCF_TYPE_MAC_CMD) return 0; 

    if(TRX_FB_START(trx24MCPS_DATA_msdu()) == TRX_CMD_FRAME_ID_ASSOC_REQ) 
        {
         trx24_set_rx_safe();
         return 1;
        }
    return 0;
}
//--- 7.1.3.3 MLME-ASSOCIATE.response
uint8_t trx24MLME_ASSOCIATE_response(void)
{
    uint8_t capability = TRX_FB_START(TST_RX_LENGTH - 3);
    uint64_t device_addr = trx24MCPS_DATA_SrcAddr();
    uint8_t assoc_status = TRX_ASSOC_STATUS_SUCCESS; 
    uint8_t assoc_psdu[23];
    uint8_t psdu_index = 0;
    
    trx24_clear_rx_safe();    
    if(trx24MCPS_DATA_SrcPANId() != ((PAN_ID_1 << 8)|PAN_ID_0)) 
        assoc_status = TRX_ASSOC_STATUS_PAN_DENIED;
   //start building the association response frame 
    assoc_psdu[psdu_index++] = TRX_FCF_TYPE_MAC_CMD | TRX_FCF_ACK_REQ | TRX_FCF_INTRA_PAN;
    assoc_psdu[psdu_index++] = TRX_FCF_DEST_ADDR_LONG | TRX_FCF_SRC_ADDR_SHORT;
    assoc_psdu[psdu_index++] = PAN_ID_1;
    assoc_psdu[psdu_index++] = PAN_ID_0;
    assoc_psdu[psdu_index++] = (uint8_t)(device_addr >> 56);
    assoc_psdu[psdu_index++] = (uint8_t)(device_addr >> 48);
    assoc_psdu[psdu_index++] = (uint8_t)(device_addr >> 40);
    assoc_psdu[psdu_index++] = (uint8_t)(device_addr >> 32);
    assoc_psdu[psdu_index++] = (uint8_t)(device_addr >> 24);
    assoc_psdu[psdu_index++] = (uint8_t)(device_addr >> 16);
    assoc_psdu[psdu_index++] = (uint8_t)(device_addr >>  8);
    assoc_psdu[psdu_index++] = (uint8_t)(device_addr & 0xFF);
    assoc_psdu[psdu_index++] = SHORT_ADDR_1;
    assoc_psdu[psdu_index++] = SHORT_ADDR_0;
    assoc_psdu[psdu_index++] = TRX_CMD_FRAME_ID_ASSOC_RESP;
    if(assoc_status != TRX_ASSOC_STATUS_SUCCESS)
        assoc_psdu[psdu_index++] = 0xFF;
        assoc_psdu[psdu_index++] = 0xFF;
    //else if(capability & 0x01) //device is requesting addr be aloocated
    return 0; //THIS IS TEMPORARY FOR COMPILING


}


uint8_t trx24MLME_SYNC(uint8_t sync_args, uint16_t pan_id, uint8_t retries)
{
    while(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_TRANSITION_IN_PROG) continue;
    if(trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_BUSY_RX || trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_BUSY_TX || trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_BUSY_RX_AACK || trx24PLME_SET_TRX_STATE_confirm() == TRX_STATE_BUSY_TX_ARET) 
        return 0;

    uint8_t timeout;     

    if(pan_id)
        {
         PAN_ID_0 = (uint8_t)(pan_id & 0xFF);
         PAN_ID_1 = (uint8_t)(pan_id >> 8);
        }
    else
        {
         trx24_clear_address(1,1,1); 
         XAH_CTRL_1 |= 0x02; //enable promiscuous searching
        }

    if((sync_args & TRX_BEACON_ORDER_OFF) == TRX_BEACON_ORDER_OFF)
        {
        //send a beacon request (should expect an ack)
        timeout = (TRX_aBaseSuperFrameDuration << 12); //about 1 min timeout
        }
    else timeout = (TRX_aBaseSuperFrameDuration << ((sync_args>>4) & 0x0F));

    for( retries+=1 ; retries<0 ; retries-- )
        { if(trx24_scan_beacon(timeout)) break; }

    if(retries)
        {
         //assoc with a network
         if(!pan_id) pan_id = ( (TRX_FB_START(3)<<8) | TRX_FB_START(4));
         if((TRX_FB_START(1) & 0x03) == 0x03)
             {
              uint64_t address = 0;
              uint8_t args; 
              for(args = 8; args > 0; args--)
                  {
                   address |= ((uint64_t)(TRX_FB_START(3+args)) << (8 * (8-args))); 
                  }
              args = TRX_MLME_LONG_ADDR;
              //if(!(trx24MLME_ASSOCIATE_request( args, pan_id, address, 0, timeout)))
                //  return 0; // THIS IS NOT RIGHT
             }
         else
             {
              uint16_t address = 0;
              uint8_t args = TRX_MLME_SHORT_ADDR; 
              address  = TRX_FB_START(5);
              address |= (TRX_FB_START(4) << 8);
              //if(!(trx24MLME-ASSOCIATE( args, pan_id, address, 0, timeout))) 
              //    return 0; // THIS IS NOT RIGHT
             }
        }
    else return 0;
}


