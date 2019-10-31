/**
  * @file    LorawanTP.cpp
  * @version 1.1.0
  * @author  Rafaella Neofytou
  * @brief   C++ file of the SX1276 driver module. 
  * Handles communication with the thingpilot nodes utilising the SX1276 (or compatible) modem
  */

/** Includes
 */
#include "LorawanTP.h"

/** EventQueue for dispatching events
 */
static EventQueue ev_queue(10 *EVENTS_EVENT_SIZE);

lorawan_app_callbacks_t cbs;

/**
 * Pin definitions are provided manually specific for Thing Pilot mote.
 * The pins that are marked NC are optional. It is assumed that these
 * pins are not connected until/unless configured otherwise. */
SX1276_LoRaRadio myradio(PA_7,PA_6,PA_5,PB_13,D7,PC_6,PC_7,PC_8,PB_14,PB_15,PC_9,
                            NC,NC,NC,NC,NC,NC,NC);

LoRaWANInterface lorawan(myradio);

/** Rx_buffer size
 */
uint8_t rx_buffer[51];

LorawanTP::LorawanTP(){
}
/** 
 */
LorawanTP:: ~LorawanTP(){
}

/** Join the TTN Network Server.
    *
    * @return              It could be one of these:
    *                       i)  0 sucess.
    *                      ii) A negative error code on failure. */
int LorawanTP::joinTTN()
{    
   lorawan_status_t retcode;
   retcode = lorawan.initialize(&ev_queue);
   if (retcode!=LORAWAN_STATUS_OK){
        return retcode; 
        }
   retcode=lorawan.set_device_class(CLASS_C);
    if (retcode!=LORAWAN_STATUS_OK){
        return retcode; 
        }
    retcode=lorawan.enable_adaptive_datarate();
    if (retcode != LORAWAN_STATUS_OK) {
         return retcode; 
         }
 
    cbs.events = mbed::callback(lora_event_handler);
    lorawan.add_app_callbacks(&cbs);
    retcode=lorawan.connect();
    if (retcode != LORAWAN_STATUS_OK || retcode !=LORAWAN_STATUS_CONNECT_IN_PROGRESS) {
            return retcode;
            }
    
/** Dispatch the event,if connected it will stop
    */
    ev_queue.dispatch_forever();
    ev_queue.break_dispatch();
    return LORAWAN_STATUS_OK; 
}
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
    *                       i)   0 sucess.
    *                       ii)  Number of bytes written to buffer.
    *                       iii) A negative error code on failure: */        

int LorawanTP::send_message(uint8_t port, uint8_t *payload, uint16_t length) {
    int8_t retcode=lorawan.send(port, payload, length, MSG_UNCONFIRMED_FLAG); 
    if (retcode < 0) {
        return retcode;
        } 
    ev_queue.dispatch_forever();
    return retcode;
       }

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
    
int64_t LorawanTP::receive_message(){
    int64_t decimalValue=0;
    uint8_t port=0;
    int flags=0;
    ev_queue.dispatch_forever();
    memset(rx_buffer, 0, sizeof(rx_buffer));
    int64_t retcode = lorawan.receive(rx_buffer, sizeof(rx_buffer), port, flags);
    if (retcode<0){
       ev_queue.break_dispatch();
       return retcode; 
       }
/** Return the result from hex to decimal
    */
    else{
        int bytes_to_buffer=retcode;
        decimalValue=rx_buffer[bytes_to_buffer-1];
        for (int i = 0; i < (retcode-1); i++) {
                int shift_bytes=(8*(bytes_to_buffer-1));
                decimalValue |=(rx_buffer[i]<<shift_bytes);
                bytes_to_buffer--;
            }
        memset(rx_buffer, 0, sizeof(rx_buffer));
        ev_queue.break_dispatch();
        return decimalValue;
      } 
}

/** Put the RF module in sleep mode & lorawan disconnect the current session..
    *
    * @return              It could be one of these:
    *                       i) LORAWAN_STATUS_DEVICE_OFF= -1011 on success
    *                       ii) A negative error code on failure. */
int LorawanTP::sleep(){
    ev_queue.break_dispatch();
    myradio.sleep();
    int retcode=lorawan.disconnect();
    return retcode;
    }

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

void LorawanTP::lora_event_handler(lorawan_event_t event)
{
    switch (event) {
        case CONNECTED:
        case DISCONNECTED:
        case TX_DONE:
        case TX_TIMEOUT:
        case TX_ERROR:
        case TX_CRYPTO_ERROR:
        case TX_SCHEDULING_ERROR:
        case RX_DONE:
        case RX_TIMEOUT:
        case RX_ERROR:
        case JOIN_FAILURE:
        case UPLINK_REQUIRED:
            ev_queue.break_dispatch();
            break;
        default:
            MBED_ASSERT("Unknown Event");
    }
}



   

