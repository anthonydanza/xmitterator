#ifndef _TRX_24_H_   
#define _TRX_24_H_   
/*---------- AVR 2.4GHz TRANCEIVER ---------------*/
/* j.telatnik                                2010 */
/* ---------------------------------------------- */
/* This library was written to take care of most  */
/* functionality of atmel based 2.4GHz IEEE       */
/* 802.15.4 tranceiver. Currently this library    */
/* only supports the ATMEGA128RFA1.               */
/*                                                */
/*   NOTES:                                       */
/*         i) Pay attention to the state of the   */
/*            radio when calling a function, if   */
/*            the radio is in an incorrect state, */
/*            the function will return an error.  */
/*            Functions with an 'f' preceding the */
/*            name will force the tranceiver into */
/*            the required initial state. ie:     */
/*            ftrx24_sleep() will force the radio */
/*            to sleep regardless of the current  */
/*            state.                              */
/*        ii) trx rate is 62.5 ksymbols/sec       */
/*                                                */
/*                                                */
/*                                                */
/*   TO DOS:                                      */
/*      1-a ) add a timer to set up auto-sending  */
/*            of the beacon based on the timing   */
/*            params in the beacon/superframe     */
/*            order                               */
/*      1-b ) add a timer for syncing to the      */
/*            beacon for non-coordinator devices  */
/*            to enable sleep/wake timing         */
/*                                                */
/*         2) make sure to actually send the      */
/*            beacon                              */
/*         3) 802.15.4 functionality              */
/*           i  ) Clear channel assesment funct   */
/*           ii ) PAN id conflict detect/resolve  */
/*           iii) PAN  association/deassociation  */
/*           iv ) random num gen for seqence nums */
/*           v  ) compress source/dest PAN-ID in  */
/*                addressing feild in FCF         */
/*         4) reconsider the naming scheme        */
/*                                                */
/*         5) rework the request/confirm paradigm */
/*                                                */
/*                                                */
/*                                                */
/*                                                */
/* ---------------------------------------------- */

//MAC Sublayer Constants defines 

#define TRX_aNumSuperFrameSlots          16
#define TRX_aBaseSlotDuration            60
#define TRX_aBaseSuperFrameDuration      (uint32_t)(TRX_aBaseSlotDuration* TRX_aNumSuperFrameSlots)



// uncategorized macros


#define trx24_set_rx_safe()        TRX_CTRL_2 |=   (uint8_t)0x80
#define trx24_clear_rx_safe()      TRX_CTRL_2 &= (uint8_t)~(0x80)

#define trx24_set_slotted()        XAH_CTRL_0 |=   (uint8_t)0x01
#define trx24_clear_slotted()      XAH_CTRL_0 &= (uint8_t)~(0x01)

#define trx24_set_auto_crc()       TRX_CTRL_1 |=   (uint8_t)0x20
#define trx24_clear_auto_crc()     TRX_CTRL_1 &= (uint8_t)~(0x20)

#define trx24_filter_res_frames()  XAH_CTRL_1 |=   (uint8_t)0x30
#define trx24_process_res_frames() XAH_CTRL_1 |=   (uint8_t)0x10
#define trx24_hide_res_frames()    XAH_CTRL_1 &= (uint8_t)~(0x30)

#define trx24_disable_ACK()        CSMA_SEED_1 |=   (uint8_t)0x10
#define trx24_enable_ACK()         CSMA_SEED_1 &= (uint8_t)~(0x10)

#define trx24_CRC_valid()          (uint8_t)(PHY_RSSI & 0x80)

#define trx24_set_dis_rx()         RX_SYN     |=   (uint8_t)0x80
#define trx24_clear_dis_rx()       RX_SYN     &= (uint8_t)~(0x80)

// Frame buffer access macros 
#define TRX_FB_START(offset)       _SFR_MEM8(0x180 + (offset))
#define TRX_FB_END(offset)         _SFR_MEM8(0x1FF - (offset))

//Defines for using the symbol counter and creating timestamps
#define trx24_sc_enable()          SCCR0 |=   (uint8_t)0x20
#define trx24_sc_disable()         SCCR0 &= (uint8_t)~(0x20)

#define trx24_beacon_timestamp()   SCCR0 |=   (uint8_t)0x40

#define trx24_sc_sel_RTC()         SCCR0 |=   (uint8_t)0x10
#define trx24_sc_sel_XOSC()        SCCR0 &= (uint8_t)~(0x10)

#define trx24_en_autotimestamp()   SCCR0 |=   (uint8_t)0x08
#define trx24_dis_autotimestamp()  SCCR0 &= (uin8_t)~(0x08)

#define trx24_set_relative_compare(n)    SCCR0 |= (uint8_t)(0x01 << (n-1))

//Defines and macros for tranceiver interupts
#define TRX_IRQ_PLL_LOCK             0x01
#define TRX_IRQ_PLL_UNLOCK           0x02
#define TRX_IRQ_RX_START             0x04
#define TRX_IRQ_RX_END               0x08
#define TRX_IRQ_CCA_ED_DONE          0x10
#define TRX_IRQ_AMI                  0x20
#define TRX_IRQ_TX_END               0x40
#define TRX_IRQ_AWAKE                0x80

#define trx24_set_IRQ(n)           if(IRQ_STATUS & n) IRQ_STATUS |= (uint8_t)(n);\
                                   IRQ_MASK |= (uint8_t)(n)
#define trx24_clear_IRQ(n)         IRQ_MASK &= (uint8_t)(~n)

//Defines and macros for symbol counter inturrupts
#define TRX_SCI_CP1                  0x01
#define TRX_SCI_CP2                  0x02
#define TRX_SCI_CP3                  0x04
#define TRX_SCI_OF                   0x08
#define TRX_SCI_BO                   0x10

#define trx24_set_SCIRQ(n)           if(SCIRQS & n) SCIRQS |= (uint8_t)(n);\
                                     SCIRQM |= (uint8_t)(n)
#define trx24_clear_SCIRQ(n)         SCIRQM &= (uint8_t)(~n)

//------------------------------------------------------------------
//                       GENERAL COMMANDS 
//------------------------------------------------------------------
//-- Set the state of the radio through the TRX_CMD bits in trx state
extern uint8_t trx24_set_state(uint8_t state_cmd);
/****
 * TRX_STATUS depends on state_cmd 
 *
 *  (note)     this command waits for the TRX_STATUS to finish any 
 *             transitions in progress before returning**
 *             **if the tranceiver is in a BUSY_(RX/TX) state, the
 *             command is written, and the state transition waits until
 *             the RX/TX is complete; however the FORCE_... commands 
 *             will inturrupt any TX/RX in progress and change state
 *
 *  state_cmd  use one of the above TRX_STATE_<value> arguments to send
 *
 *  returns    1 if state has changed as requested, 0 if unsuccesful or
 *             initial state was invalid
 ****/

//ARGS for initing the device
#define TRX_INIT_NORMAL_DEVICE           0x0000
#define TRX_INIT_PAN_COORD               0x0001
#define TRX_INIT_PROMISCUOUS             0x0002
#define TRX_INIT_SNIFFER                 0x0003
#define TRX_INIT_AACK                    0x0004
#define TRX_INIT_ARET                    0x0008
#define TRX_INIT_SLOTTED                 0x0010
#define TRX_INIT_RX_SAFE_MODE            0x0020
#define TRX_INIT_TX_AUTO_CRC             0x0040
#define TRX_INIT_RECV_RES_FRAMES         0x0080

//--- Trn on power to the tranceiver and wake it up, setup registers
extern uint8_t trx24_init(uint16_t init_args, uint8_t trx_channel);
/****
 * TRX_STATUS = TRX_OFF| PLL_ON | SLEEP
 *   SLEEP --> TRX_OFF; else unchanged
 *
 *
 *
 ****/

//--- Sets any non-zero address, to clear an address use trx_24_clear_addr()
extern uint8_t trx24_set_address(uint16_t short_addr, uint16_t pan_id, uint64_t long_addr);
/****
 * TRX_STATUS = TRX_OFF | PLL_ON
 *   --> unchanged 
 *
 *  any non-zero feild will write that value to the corresponding address
 *   ie: trx_set_address(0x0123, 0 , 0);
 *      will cause the short address to be changed to 0x0123, and keep the 
 *      remaining addresses unchanged.
 *
 *  returns    0 if unsuccessful, 1 if successful
 ****/

//--- any set arguments will be cleared (set to 0's)
extern uint8_t trx24_clear_address(uint8_t short_addr, uint8_t pan_id, uint8_t long_addr);
/****
 * TRX_STATUS = TRX_OFF | PLL_ON
 *   --> unchanged 
 *   
 *  any non-zero feild causes the corresponding address to be cleared
 *  ie: trx24_clear_address(1,0,1);
 *   will cause the short and long adresses to be cleared, and keep the 
 *   PAN-ID unchanged 
 *
 *  returns    0 if unsuccessful, 1 if successful
 ****/

//--- Write a byte of data to the frame buffer with offset from TRXFBST or TRXFBEND
extern uint8_t trx24_write(uint8_t data, int8_t offset);
/****
 * TRX_STATUS = TRX_OFF | PLL_ON 
 * 
 *  data       data (octet) to be written        
 *  offset     offset from the start (if positive) or end (if negative)   
 *             of the frame buffer
 *  returns    0 if unsuccessful, 1 if successful
 ****/


//--- Write an array data[len] to the frame buffer with offset from TRXFBST or TRXFBEND
extern uint8_t trx24_writel(uint8_t *data, uint8_t len, int8_t offset);
/****
 * TRX_STATUS = TRX_OFF | PLL_ON 
 * 
 * *data       points to the first element ofthe array to be written 
 *  len        length of the array in octets   
 *  offset     offset from the start (if positive) or end (if negative)   
 *             of the frame buffer
 *  returns    0 if unsuccessful, 1 if successful
 ****/


//------------------------------------------------------------------
//                        DEVICE COMMANDS 
//------------------------------------------------------------------


/****
 * TRX_STATUS = TRX_OFF | SLEEP | RX_ON | PLL_ON
 *   --> PLL_ON
 *
 * assoc_args   short/long addr of beacon coord
 * pan_id       PAN_ID of the coordinator, if 0; use PAN_ID_n registers
 * address      address of the coordinator
 * channel      logical channel of the coord, 
 *              if 0, use channel currently in PHY_CC_CCA register
 * timeout      symbols to wait, before returning an unsuccess
 *              this should reflect the BeaconOrder of the PAN coordinator
 *              if 0, set to a default of BecaonOrder == 14
 *
 * return       1 is successful, 0 if could not assoc with the selected PAN 
 *              
 *
 ****/


//--- Scan for a PAN Coordinator beacon, save beacon to the frame buffer, Adressing Fields can be read to configure the device to a PAN
extern uint8_t trx24_scan_beacon(uint32_t timeout);
/****
 * TRX_STATUS = TRX_OFF | SLEEP | RX_ON | PLL_ON
 *   --> PLL_ON
 *
 * timeout      symbols to wait, before returning a not found
 *              if 0, timesout is set to 1 min (3720000UL)
 *
 * return       1 is successful, 0 if no beacons found 
 *              
 *
 ****/

// SEND ARGS


//--- try to sync with a PAN-beacon on a PAN network, this auto sets up addressing registers and sets the timeout registers 
/****
 * TRX_STATUS = TRX_OFF | SLEEP | RX_ON | PLL_ON
 *   --> PLL_ON
 *
 * sync-args    accepts TRX_BEACON_ORDER_## defines listed under the 
 *              trx_pan_beacon() command
 *              if BEACON_ORDER_[0-14] waits baselength*(2^BO + 1) 
 *              if BEACON_ORDER_15 boadcast/send request for a PAN beacon
 * pan-id       if provided, attempts to sync with coordinator on this 
 *              PAN-ID; if 0 scans all beacons and syncs first coord found
 * retries      maximum number of times to wait before returning 0 
 *
 *
 * return       1 if successful, 0 if no matching beacons found after retries
 *              
 *
 ****/

//------------------------------------------------------------------
//                    PAN COORDINATOR COMMANDS
//------------------------------------------------------------------

//BEACON_ARGS
#define TRX_BEACON_SHORT_ADDR       0x00   //use short, 16-bit, addresses <- default
#define TRX_BEACON_LONG_ADDR        0x01   //use long, 64-bit address 

#define TRX_OFF_INACTIVE_T          0x00   //(not implemented)TRX_OFF during inactive periods <-default
#define TRX_SLEEP_INACTIVE_T        0x02   //(not implemented)sleep during inactive periods,

#define TRX_BEACON_ORDER_0          0x00   // (60*16) symbols (15.36 ms) between beacon transmissions 
#define TRX_BEACON_ORDER_1          0x10   // (60*16) * 2^1 symbols (30.72 ms) 
#define TRX_BEACON_ORDER_2          0x20   // (60*16) * 4 symbols     (61.44 ms)
#define TRX_BEACON_ORDER_3          0x30   // (60*16) * 8 between     (~123 ms)
#define TRX_BEACON_ORDER_4          0x40   // (60*16) * 16 between    (~246 ms)
#define TRX_BEACON_ORDER_5          0x50   // (60*16) * 32 symbols    (~492 ms)
#define TRX_BEACON_ORDER_6          0x60   // (60*16) * 64 symbols    (~98.3 s)
#define TRX_BEACON_ORDER_7          0x70   // (60*16) * 128 symbols   (~1.97 s)
#define TRX_BEACON_ORDER_8          0x80   // (60*16) * 256 symbols   (3.93 s)
#define TRX_BEACON_ORDER_9          0x90   // (60*16) * 512 symbols   (7.86 s)
#define TRX_BEACON_ORDER_10         0xA0   // (60*16) * 1024 symbols  (15.7 s)
#define TRX_BEACON_ORDER_11         0xB0   // (60*16) * 2048 symbols  (31.5 s)
#define TRX_BEACON_ORDER_12         0xC0   // (60*16) * 4096 symbols  (62.9 s)
#define TRX_BEACON_ORDER_13         0xD0   // (60*16) * 8192 symbols  (126 s)
#define TRX_BEACON_ORDER_14         0xE0   // (60*16) * 16384 symbols (252 s)
#define TRX_BEACON_ORDER_OFF        0xF0   // no timed beacons
#define TRX_BEACON_ORDER_15         0xF0   // no timed beacons

//ensure SFRAME_ORDER <= BEACON_ORDER
#define TRX_BEACON_SFRAME_ORDER_0   0x0000   //Receiver is only active during a 
              // superframe duration of 16 slots at a slot length of 60 * (2^0) symbols
#define TRX_BEACON_SFRAME_ORDER_1   0x0100   // (60*16) * 2^1 superframe duration
#define TRX_BEACON_SFRAME_ORDER_2   0x0200   // (60*16) * 4 superframe duration
#define TRX_BEACON_SFRAME_ORDER_3   0x0300   // (60*16) * 8 superframe duration
#define TRX_BEACON_SFRAME_ORDER_4   0x0400   // (60*16) * 16 superframe duration
#define TRX_BEACON_SFRAME_ORDER_5   0x0500   // (60*16) * 32 superframe duration
#define TRX_BEACON_SFRAME_ORDER_6   0x0600   // (60*16) * 64 superframe duration
#define TRX_BEACON_SFRAME_ORDER_7   0x0700   // (60*16) * 128 superframe duration
#define TRX_BEACON_SFRAME_ORDER_8   0x0800   // (60*16) * 256 superframe duration
#define TRX_BEACON_SFRAME_ORDER_9   0x0900   // (60*16) * 512 superframe duration
#define TRX_BEACON_SFRAME_ORDER_10  0x0A00   // (60*16) * 1024 superframe duration
#define TRX_BEACON_SFRAME_ORDER_11  0x0B00   // (60*16) * 2048 superframe duration
#define TRX_BEACON_SFRAME_ORDER_12  0x0C00   // (60*16) * 4096 superframe duration
#define TRX_BEACON_SFRAME_ORDER_13  0x0D00   // (60*16) * 8192 superframe duration
#define TRX_BEACON_SFRAME_ORDER_14  0x0E00   // (60*16) * 16384 superframe duration
#define TRX_BEACON_SFRAME_ORDER_OFF 0x0F00   // superframe structure not used          


//--- Transmit a beacon for superframe, start listening 

extern uint8_t trx24_pan_beacon(uint16_t beacon_args, uint8_t *beacon_payload, uint8_t beacon_payload_length);
/****
 * TRX_STATUS = TRX_OFF | SLEEP | RX_ON | PLL_ON
 *   --> RX_ON 
 *
 *  notes:    i) beacons will be sent off the SFRAME_ORDER provided to 
 *               beacon_args, timing uses SCCMP1 in relative mode and the 
 *               sent beacon timestamp is stored in SCBTSR, if beacon order is
 *               off, no timing is used, frame sent immediately
 *           ii) make sure the symbol counter is running for proper operation
 *          iii) to send a beacon immediately, set SCBTSR = 0
 *           iv) once called this will wait until the correct frame order
 *
 *
 *  beacon_args         Combination of ANDed TRX_BEACON_... arguments, 0x00's indicate 
 *                      defaults
 * *beacon_payload      points to first element in an array of a payload to attach
 *                      to the beacon, up to 32 octets
 *  beacon_payload_len  length of the beacon_payload array in octets
 *
 *  return              beacon sequence number if successful, 0 if unsuccessful
 *                      0x01 -> 0xFF
 ****/

/*________________________________________________________________
 |                                                                |
 |                     PHY Service                                |
 |________________________________________________________________*/

#define TRX_aMaxPHYPacketSize                             127

//  6.2.1 PHY Data Service
//================================

//--- 6.2.1.1 PD-DATA.request
//   The PD-DATA.request primitive requests the transfer of an MPDU 
//   (i.e., PSDU) from the MAC sublayer to the local PHY entity.
extern uint8_t trx24PD_DATA(uint8_t psduLength, uint8_t *psdu);
/***
 *  initial state must be [PLL_ON|TX_ARET_ON] >> return state is unchanged 
 *
 *  psduLength      Number of octets contained in the PSDU to be transmitted to 
 *                  the PHY ( psdu <= TRX_aMaxPHYPacketSize )
 *  *psdu           pointer to the first element of an array of octects forming 
 *                  the PSDU to be transmitted to the PHY
 *
 *  return          1 if successful, 0 if unsuccessful
 ***/

//--- 6.2.1.2 PD-DATA.confirm
 //    !NOT IMPLEMENTED
//--- 6.2.1.3 PD-DATA.indication
//   The PD-DATA.indication primitive indicates the transfer of an MPDU 
//   (i.e., PSDU) from the PHY to the local MAC sublayer entity.
#define trx24PD_DATA_psduLength()                TST_RX_LENGTH
#define trx24PD_DATA_ppduLinkQuality()           TRX_FB_START(TST_RX_LENGTH)


//  6.2.2 PHY Management Service
//================================

//--- 6.2.2.1 PLME-CCA.request 
 //    !NOT IMPLEMENTED
//--- 6.2.2.2 PLME-CCA.confirm 
 //    !NOT IMPLEMENTED
//--- 6.2.2.3 PLME-ED.request 
//   The PLME-ED.request primitive requests that the PLME perform 
//   an ED measurement
#define trx24PLME_ED()                           PHY_ED_LEVEL

//--- 6.2.2.5 PLME-GET.request
//   The PLME-GET.request primitive requests information about a given
//   PHY PIB attribute.
#define trx24PLME_GET_phyCurrentChannel()             (PHY_CC_CCA & 0x1F) 
 //    !GET OF OTHER PIB ATTRIBUTES NOT IMPLEMENTED

//--- 6.2.2.7 PLME-SET-TRX-STATE.request
//   The PLME-SET-TRX-STATE.request primitive requests that the PHY entity 
//   change the internal operating state of the transceiver.
#define TRX_STATE_TX_START             0x02
#define TRX_STATE_FORCE_TRX_OFF        0x03
#define TRX_STATE_FORCE_PLL_ON         0x04 
#define TRX_STATE_RX_ON                0x06
#define TRX_STATE_TRX_OFF              0x08
#define TRX_STATE_PLL_ON               0x09
#define TRX_STATE_TX_ON                0x09
#define TRX_STATE_RX_AACK_ON           0x16
#define TRX_STATE_TX_ARET_ON           0x19
#define TRX_STATE_SLEEP                0x0F
extern uint8_t trx24PLME_SET_TRX_STATE(uint8_t state);
/****
 *  (note)     this command waits for the TRX_STATE to finish any 
 *             transitions in progress before returning**
 *             **if the tranceiver is in a BUSY_(RX/TX) state, the
 *             command is written, and the state transition waits until
 *             the RX/TX is complete; however the FORCE_... commands 
 *             will inturrupt any TX/RX in progress and change state
 *
 *  state      use one of the above TRX_STATE_<value> arguments to send
 *
 *  returns    1 if state has changed as requested, 0 if unsuccesful or
 *             initial state was invalid
 ****/

//--- 6.2.2.8 PLME-SET-TRX-STATE.confirm
//   The PLME-SET-TRX-STATE.confirm primitive reports the result of a 
//   request to change the internal operating state of the transceiver.
#define TRX_STATE_BUSY_RX              0x01
#define TRX_STATE_BUSY_TX              0x02
#define TRX_STATE_BUSY_RX_AACK         0x11
#define TRX_STATE_BUSY_TX_ARET         0x12
#define TRX_STATE_TRANSITION_IN_PROG   0x1F
//also, see the above states in 6.2.2.7
#define trx24PLME_SET_TRX_STATE_confirm()     (uint8_t)(TRX_STATUS & 0x0F)
#define trx24PLME_GET_TRX_STATE()             (uint8_t)(TRX_STATUS & 0x0F)


//--- 6.2.2.9 PLME-SET.request
//   The PLME-SET.request primitive attempts to set the indicated PHY PIB 
//   attribute to the given value.
#define trx24PLME_SET_phyCurrentChannel(chan) PHY_CC_CCA = ((PHY_CC_CCA & 0xE0)|((uint8_t)(chan)))  
 //    !SET OF OTHER PIB ATTRIBUTES NOT IMPLEMENTED

//--- 6.2.2.10 PLME-SET.confirm
 //    !NOT IMPLEMENTED; use PLME-GET.request instead

/*________________________________________________________________
 |                                                                |
 |                    MAC Sublayer Specification                  |
 |________________________________________________________________*/

//  7.1.1 MAC Data Service
//================================

//--- 7.1.1.1 MCPS-DATA.request
//   The MCPS-DATA.request primitive requests the transfer of a data SPDU 
//   (i.e., MSDU) from a local SSCS entity to a single peer SSCS entity.
#define TRX_SEND_SECURE_EN         0x10
//TxOptions
#define TRX_SEND_FRAME_PENDING     0x08
#define TRX_SEND_EXPECT_ACK        0x04
#define TRX_SEND_INTRAPAN          0x02
//SrcAddrMode
#define TRX_SEND_SRC_SHORT_ADDR    0x00
#define TRX_SEND_SRC_LONG_ADDR     0x40
#define TRX_SEND_DEST_SHORT_ADDR   0x00
#define TRX_SEND_DEST_LONG_ADDR    0x20
#define TRX_SEND_TX_ARET_ON        0x80
extern uint8_t trx24MCPS_DATA(uint8_t *msdu, uint8_t msduLength, uint8_t sequence_num, uint8_t tx_args, uint16_t DstPANId, uint64_t DstAddr);
/***
 * TRX_STATUS = TRX_OFF | PLL_ON | TX_ARET_ON
 *   --> PLL_ON or TX_ARET_ON
 *
 * *msdu         Pointer to the first element of an array of octets forming the
 *               MSDU to be transmitted.
 * msduLength    Number of octets contained in the MSDU to be transmitted
 * sequence_num  Data sequence number
 * tx_args       bitwise OR'ed arguments using TRX_SEND_... arguments
 * DstPANId      16-bit PAN identifier of the entity to which the MSDU is being 
 *               transmitted.
 * DstAddr       16 or 32-bit device address of the entity to which the MSDU is
 *               being transmitted.
 *
 * return         1 if successful, 0 if unsuccessful
 ***/

//--- 7.1.1.2 MCPS-DATA.confirm
// The MCPS-DATA.confirm primitive reports the results of a request to transfer
// a data SPDU (MSDU) from a local SSCS entity to a single peer SSCS entity.
extern uint32_t trx24MCPS_DATA_Timestamp(void);         
/***
 * return         Time, in symbols, at which the data was transmitted.
 ***/

//--- 7.1.1.3 MCPS-DATA.indication
//   The MCPS-DATA.indication primitive indicates the transfer of a data SPDU
//   (i.e., MSDU) from the MAC sublayer to the local SSCS entity.

#define TRX_ADDR_MODE_SHORT                    0x02
#define TRX_ADDR_MODE_LONG                     0x03
#define TRX_ADDR_MODE_NONE                     0x00

#define trx24MCPS_DATA_SrcAddrMode()           (uint8_t)(TRX_FB_START(1) & 0x03)
extern uint16_t trx24MCPS_DATA_SrcPANId(void);      
extern uint64_t trx24MCPS_DATA_SrcAddr(void);      
#define trx24MCPS_DATA_DstAddrMode()           (uint8_t)((TRX_FB_START(1) & 0x30)>>4)
extern uint16_t trx24MCPS_DATA_DstPANId(void);      
extern uint64_t trx24MCPS_DATA_DstAddr(void);      
extern uint8_t  trx24MCPS_DATA_msduLength(void);        
extern uint8_t  trx24MCPS_DATA_msdu(void);        
#define trx24MCPS_DATA_mpduLinkQuality()        TRX_FB_START(TST_RX_LENGTH)
//for trx24MCPS_DATA_Timestamp() see 7.1.1.2

//--- 7.1.1.4 MCPS-PURGE.request
 //    !NOT IMPLEMENTED
//--- 7.1.1.5 MCPS-PURGE.confirm
 //    !NOT IMPLEMENTED

//  7.1.3 Association Primatives
//================================

#define TRX_MLME_LONG_ADDR           0x00
#define TRX_MLME_SHORT_ADDR          0x01

//--- 7.1.3.1 MLME-ASSOCIATE.request
//   The MLME-ASSOCIATE.request primitive allows a device to request an
//   association with a coordinator.
//CoordAddrMode
#define TRX_MLME_ADDR_MODE_LONG      0X01
#define TRX_MLME_ADDR_MODE_SHORT     0x00
//CapabilityInformation
#define TRX_MLME_CI_ALT_PAN          0X08
#define TRX_MLME_CI_FFD              0X10
#define TRX_MLME_CI_MAINS_PWR        0X20
#define TRX_MLME_CI_RX_ON_IDLE       0X40
#define TRX_MLME_CI_ALLOC_ADDR       0X80
extern uint8_t trx24MLME_ASSOCIATE_request(uint8_t assoc_args, uint8_t LogicalChannel, uint16_t CoordPANId, uint64_t CoordAddress);
/***
 * TRX_STATUS = PLL_ON | TX_ARET_ON
 *   --> PLL_ON 
 *
 *  (note)         PAN_ID and long addr of this device should already be set
 *  assoc_args     Bitwise OR'ed arguments for CoordAddrMode, 
 *                 CapabilityInformation.
 *  LogicalChannel The logical channel on which to attempt association.
 *                 (LogicalChannel = 0 will not change from current chan)
 *  CoordPANId     Identifier of the PAN on which to associate.
 *  CoordAddress   The address of the coordinator with which to associate.
 *
 *  return         1 if association was successful, 0 if failed.
 ***/
//--- 7.1.3.2 MLME-ASSOCIATE.indication
//   The MLME-ASSOCIATE.indication primitive is generated by the MLME of the
//   coordinator and issued to its next higher layer to indicate the
//   reception of an association request command
extern uint8_t trx24MLME_ASSOCIATE_indication(void);
/***
 *  return         1 if received frame was an ASSOC_REQ command frame,
 *                 otherwise 0
 ***/
//--- 7.1.3.3 MLME-ASSOCIATE.response
//   The MLME-ASSOCIATE.response primitive is generated by the next higher
//   layer of a coordinator and issued to its MLME in order to respond to
//   the MLME-ASSOCIATE.indication primitive.
extern uint8_t trx24MLME_ASSOCIATE_response(void);
/***
 *  return         1 if the device requesting assoc was associated to the PAN
 *                 0 if the device was not associated
 ***/

//--- 7.1.3.4 MLME-ASSOCIATE.confirm

extern uint8_t trx24MLME_SYNC(uint8_t sync_args, uint16_t pan_id, uint8_t retries);


//  7.2   MAC Frame Formats    
//================================

//--- 7.2.1.1 Frame Control Feild    
#define TRX_FCF_TYPE_BEACON             0x00
#define TRX_FCF_TYPE_DATA               0x20
#define TRX_FCF_TYPE_ACK                0x40
#define TRX_FCF_TYPE_MAC_CMD            0x60
#define TRX_FCF_FRAME_PENDING           0x08
#define TRX_FCF_ACK_REQ                 0x04
#define TRX_FCF_INTRA_PAN               0x02
#define TRX_FCF_DEST_ADDR_LONG          0x30
#define TRX_FCF_DEST_ADDR_SHORT         0x20
#define TRX_FCF_SRC_ADDR_LONG           0x03
#define TRX_FCF_SRC_ADDR_SHORT          0x02

//--- 7.2.2.4 MAX command frame format    
#define TRX_CMD_FRAME_ID_ASSOC_REQ        0x01
#define TRX_CMD_FRAME_ID_ASSOC_RESP       0x02
#define TRX_CMD_FRAME_ID_DISASSOC_NOTICE  0x03
#define TRX_CMD_FRAME_ID_DATA_REQ         0x04
#define TRX_CMD_FRAME_ID_PANID_CONFLICT   0x05
#define TRX_CMD_FRAME_ID_ORPHAN_NOTICE    0x06
#define TRX_CMD_FRAME_ID_BEACON_REQ       0x07
#define TRX_CMD_FRAME_ID_COORD_REALIGN    0x08
#define TRX_CMD_FRAME_ID_GTS_REQ          0x09

//  7.3   MAC Command Frames    
//================================

//--- 7.3.2.3 Association response > status feild
#define TRX_ASSOC_STATUS_SUCCESS          0x00
#define TRX_ASSOC_STATUS_PAN_FULL         0x01
#define TRX_ASSOC_STATUS_PAN_DENIED       0x02



#endif
