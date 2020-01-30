/**
  * @file    LorawanTP.h
  * @version 0.3.0
  * @author  Rafaella Nofytou
  * @brief   Header file of the SX1276 driver module. 
  * Handles communication with the thingpilot nodes utilising the SX1276 (or compatible) modem
  */

/** Define to prevent recursive inclusion
 */
#pragma once

/** Includes 
 */
#include "mbed.h"
#include <cstdio>
#include "lorawan/LoRaRadio.h"
#include "SX1276_LoRaRadio.h"
#include "lorawan/LoRaWANInterface.h"
#include "lorawan/system/lorawan_data_structures.h"
#include "events/EventQueue.h"
#include "credentials.h"

#define CLOCK_SYNCH_ACK_PORT   220
#define CLOCK_SYNCH_PORT 221
#define SCHEDULER_PORT 222
#define RESET_PORT  223


/** Base class for the ThingPilot LorawanTP interface
 */ 
class LorawanTP 
{
    public: 
/** Constructor for the LorawanTP class
 */
    LorawanTP(PinName mosi,PinName miso,PinName sclk,PinName nss,PinName reset,PinName dio0,PinName dio1,PinName dio2,
                     PinName dio3,PinName dio4,PinName dio5,PinName rf_switch_ctl1,PinName rf_switch_ctl2,PinName txctl,
                     PinName rxctl,PinName ant_switch,PinName pwr_amp_ctl,PinName tcxo);

/** Destructor for the LorawanTP class
 */
    ~LorawanTP();

    /** Join the TTN Network Server with Class A or C.
    *
    * @param device_class   Currently Only Support CLASS_A or CLASS_C, CLASS_B is not yet supported 
    * @return               It could be one of these:
    *                       i)  0 sucess.
    *                      ii) A negative error code on failure. */
    int join(const device_class_t device_class);
    
    /** Send a message from the Network Server on a specific port.
    *
    * @param port          The application port number. Port numbers 0 and 224 are reserved,
    *                      whereas port numbers from 1 to 223 (0x01 to 0xDF) are valid port numbers.
    *                      Anything out of this range is illegal.
    *
    * @param payload       A buffer with data from the user or stored in eeproom.
    *
    * @param length        The size of data in bytes.
    * @param flags         A flag is used to determine what type of message is being sent, for example:
    *
    *                      MSG_UNCONFIRMED_FLAG = 0x01
    *                      MSG_CONFIRMED_FLAG   = 0x02
    *                      MSG_MULTICAST_FLAG   = 0x04
    *                      MSG_PROPRIETARY_FLAG = 0x08
    *
    *
    * @return              It could be one of these:
    *                       i)  Number of bytes send on sucess.
    *                       ii) A negative error code on failure
    *                      LORAWAN_STATUS_NOT_INITIALIZED   if system is not initialized with initialize(),
    *                      LORAWAN_STATUS_NO_ACTIVE_SESSIONS if connection is not open,
    *                      LORAWAN_STATUS_WOULD_BLOCK       if another TX is ongoing,
    *                      LORAWAN_STATUS_PORT_INVALID      if trying to send to an invalid port (e.g. to 0)
    *                      LORAWAN_STATUS_PARAMETER_INVALID if NULL data pointer is given or flags are invalid
*/        
    int send_message(uint8_t port, uint8_t payload[], uint16_t length);

    /** Receives a message from the Network Server on a specific port.
    *
    * @param port          The application port number. Port numbers 0 and 224 are reserved,
    *                      whereas port numbers from 1 to 223 (0x01 to 0xDF) are valid port numbers.
    *                      Anything out of this range is illegal.
    *
    * @param data          A pointer to buffer where the received data will be stored.
    *
    * @param length        The size of data in bytes.
    *
    * @param flags         A flag is used to determine what type of message is being received, for example:
    *
    *                      MSG_UNCONFIRMED_FLAG = 0x01
    *                      MSG_CONFIRMED_FLAG   = 0x02
    *                      MSG_MULTICAST_FLAG   = 0x04
    *                      MSG_PROPRIETARY_FLAG = 0x08
    *
    * @return              It could be one of these:
    *                       i)   0 if there is nothing else to read.
    *                       ii)  Number (decimal value) of bytes written to user buffer.
    *                       iii) A negative error code on failure. */
    int receive_message(uint32_t* rx_dec_buffer, uint8_t& rx_port, int& rx_retcode);


    int get_unix_time(uint32_t& unix_time);

    /** Put the RF module in sleep mode & lorawan disconnect the current session..
    *
    * @return              It could be one of these:
    *                       i)LORAWAN_STATUS_OK (the statuses are reversed-simplicity reasons) on sucessfull disconnection,
    *                       ii) A negative error code (-1011) on failure to disconeect . */
    int sleep();
    
    private:
    /**
    * Events needed to announce stack operation results.
    *
    * CONNECTED            - When the connection is complete
    * DISCONNECTED         - When the protocol is shut down in response to disconnect()
    * TX_DONE              - When a packet is sent
    * TX_TIMEOUT,          - When stack was unable to send packet in TX window
    * TX_ERROR,            - A general TX error
    * CRYPTO_ERROR,        - A crypto error indicating wrong keys
    * TX_SCHEDULING_ERROR, - When stack is unable to schedule packet
    * RX_DONE,             - When there is something to receive
    * RX_TIMEOUT,          - Not yet mapped
    * RX_ERROR             - A general RX error
    * JOIN_FAILURE         - When all Joining retries are exhausted
    * UPLINK_REQUIRED      - Stack indicates application that some uplink needed
    * AUTOMATIC_UPLINK_ERROR - Stack tried automatically send uplink but some error occurred.
    *                          Application should initiate uplink as soon as possible.
    * @return break_dispatch. Break out of a running event loop in any case when an event is finished. 
    *
    *  Forces the specified event queue's dispatch loop to terminate. Pending
    *  events may finish executing, but no new events will be executed. */
    static void lora_event_handler(lorawan_event_t event);

	SX1276_LoRaRadio _myradio;
    LoRaWANInterface lorawan;
    
};

  