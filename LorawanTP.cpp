/**
  * @file    LorawanTP.cpp
  * @version 1.0.1
  * @author  Rafaella Neofytou
  * @brief   C++ file of the SX1276 driver module. 
  * Handles communication to the thingpilot API utilising the SX1272/SX1276 (or compatible) modem
  */

/** Includes
 */
#include "LorawanTP.h"
#include "mbed_trace.h"


static EventQueue ev_queue(10 *EVENTS_EVENT_SIZE);

lorawan_app_callbacks_t cbs;

SX1276_LoRaRadio myradio(PA_7,PA_6,PA_5,PB_13,D7,PC_6,PC_7,PC_8,PB_14,PB_15,PC_9,
                            NC,NC,NC,NC,NC,NC,NC);

LoRaWANInterface lorawan(myradio);

uint8_t tx_buffer[30];
uint8_t rx_buffer[30];

LorawanTP::LorawanTP(){
}

LorawanTP:: ~LorawanTP(){
}
/** Join the TTN Network Server.
     *
     * @return              It could be one of these:
     *                       i)  0 sucess.
     *                       ii) A negative error code on failure. */
int LorawanTP::joinTTN()
{    
   lorawan_status_t retcode;
  
   if (lorawan.initialize(&ev_queue)!=LORAWAN_STATUS_OK){
        return LORA_ERROR; }
   
    if (lorawan.set_device_class(CLASS_C)!=LORAWAN_STATUS_OK){
        return LORA_ERROR; }

    if (lorawan.enable_adaptive_datarate() != LORAWAN_STATUS_OK) {
         return LORA_ERROR; }
    
    /** Drives the application
     */
    cbs.events = mbed::callback(lora_event_handler);
    lorawan.add_app_callbacks(&cbs);
    retcode=lorawan.connect();
    if (retcode != LORAWAN_STATUS_OK || retcode !=LORAWAN_STATUS_CONNECT_IN_PROGRESS) {
            return retcode;}
    
    /** Dispatch the event,if connected it will stop
     */
    ev_queue.dispatch_forever();

    return LORAWAN_STATUS_OK; 

}
/** Send a message from the Network Server on a specific port.
     *
     * @param port          The application port number. Port numbers 0 and 224 are reserved,
     *                      whereas port numbers from 1 to 223 (0x01 to 0xDF) are valid port numbers.
     *                      Anything out of this range is illegal.
     *
     * @param payload       A buffer with data stored.
     *
     * @param length        The size of data in bytes.
     *
     * @return              It could be one of these:
     *                       i)   0 sucess.
     *                       ii)  Number of bytes written to buffer.
     *                       iii) A negative error code on failure: */
 int LorawanTP::send_message(uint8_t port, uint8_t payload[], uint16_t length) {

        int16_t retcode;  
        retcode=lorawan.send(port, payload, sizeof(*payload), MSG_UNCONFIRMED_FLAG); 
        if (retcode < 0) {
            return retcode;} 

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
     * @param flags         A flag is used to determine what type of message is being sent, for example:
     *
     *                      MSG_UNCONFIRMED_FLAG = 0x01
     *                      MSG_CONFIRMED_FLAG   = 0x02
     *                      MSG_MULTICAST_FLAG   = 0x04
     *                      MSG_PROPRIETARY_FLAG = 0x08
     *
     *                      All flags can be used in conjunction with one another depending on the intended
     *                      use case or reception expectation.
     *
     *                      For example, MSG_CONFIRMED_FLAG and MSG_UNCONFIRMED_FLAG are
     *                      not mutually exclusive. In other words, the user can subscribe to
     *                      receive both CONFIRMED AND UNCONFIRMED messages at the same time.
     *
     * @return              It could be one of these:
     *                       i)   0 if there is nothing else to read.
     *                       ii)  Number of bytes written to user buffer.
     *                       iii) A negative error code on failure:
     *                       LORAWAN_STATUS_NOT_INITIALIZED   if system is not initialized with initialize(),
     *                       LORAWAN_STATUS_NO_ACTIVE_SESSIONS if connection is not open,
     *                       LORAWAN_STATUS_WOULD_BLOCK        if there is nothing available to read at the moment,
     *                       LORAWAN_STATUS_PARAMETER_INVALID  if NULL data or length is given,
     *                       LORAWAN_STATUS_WOULD_BLOCK        if incorrect port or flags are given, */
    
    uint8_t * LorawanTP::receive_message(){
    ev_queue.dispatch_forever();
    uint8_t port;
    int flags;
    memset(rx_buffer, 0, sizeof(rx_buffer));
    int64_t retcode = lorawan.receive(rx_buffer, sizeof(rx_buffer), port, flags);
     
    if ( retcode<0){
      // a.printf("\r\nNo Messages received\r\n");
       ev_queue.break_dispatch();
       return 0;
    }
 return rx_buffer; //pointer to the rx buffer
}

/** Lora Radio sleep & lorawan disconnect.
     *
     * @return              It could be one of these:
     *                       i)  0 sucess.
     *                       ii) A negative error code on failure. */
int LorawanTP::sleep(){
       lorawan.disconnect();
       myradio.sleep();
       int retcode= myradio.get_status();
       return retcode;
    }

/** Event handler.
     *
     * @return              It could be one of these:
     *                       i)  CONNECTED.....
     *                       ii) 
     */
void LorawanTP::lora_event_handler(lorawan_event_t event)
{
    switch (event) {
        
        case CONNECTED: 
            ev_queue.break_dispatch();
            break;
        case DISCONNECTED:
            ev_queue.break_dispatch();
            break;
        case TX_DONE:
            ev_queue.break_dispatch();
            break;
        case TX_TIMEOUT:
        case TX_ERROR:
        case TX_CRYPTO_ERROR:
        case TX_SCHEDULING_ERROR:
            ev_queue.break_dispatch();
            break;
        case RX_DONE:
            ev_queue.break_dispatch();
        case RX_TIMEOUT:
            ev_queue.break_dispatch();
            break;
        case RX_ERROR:
            ev_queue.break_dispatch();
            break;
        case JOIN_FAILURE:
            ev_queue.break_dispatch();
            break;
        case UPLINK_REQUIRED:
            break;
        default:
            MBED_ASSERT("Unknown Event");
    }
}



   

