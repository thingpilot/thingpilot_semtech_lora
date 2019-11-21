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
static EventQueue ev_queue(20 *EVENTS_EVENT_SIZE);

lorawan_app_callbacks_t cbs;


/** Constructor for the LorawanTP class
 */
LorawanTP::LorawanTP(PinName mosi,PinName miso,PinName sclk,PinName nss,PinName reset,PinName dio0,PinName dio1,PinName dio2,
                     PinName dio3,PinName dio4,PinName dio5,PinName rf_switch_ctl1,PinName rf_switch_ctl2,PinName txctl,
                     PinName rxctl,PinName ant_switch,PinName pwr_amp_ctl,PinName tcxo): 
                     _myradio(mosi,miso,sclk,nss,reset,dio0,dio1,dio2,dio3,dio4,dio5,rf_switch_ctl1,rf_switch_ctl2,txctl,
                     rxctl,ant_switch,pwr_amp_ctl,tcxo), lorawan(_myradio) {
          
}

/** Destructor for the LorawanTP class
 */
LorawanTP:: ~LorawanTP(){
   _myradio.~SX1276_LoRaRadio();
}

/** Join the TTN Network Server.
    *
    * @return              It could be one of these:
    *                       i)  0 sucess.
    *                      ii) A negative error code on failure. */
int LorawanTP::join()
{   
    lorawan_status_t retcode;
    retcode = lorawan.initialize(&ev_queue);
    if (retcode!=LORAWAN_STATUS_OK){
            return retcode; 
            }
    cbs.events = mbed::callback(lora_event_handler);
    lorawan.add_app_callbacks(&cbs);
  
    retcode=lorawan.enable_adaptive_datarate();
    if (retcode != LORAWAN_STATUS_OK) {
        return retcode; 
        }

        retcode=lorawan.connect();
    if  ((retcode != LORAWAN_STATUS_OK) || (retcode !=LORAWAN_STATUS_CONNECT_IN_PROGRESS ||retcode!=LORAWAN_STATUS_ALREADY_CONNECTED)) {
        retcode=lorawan.connect(); 
       // return retcode;
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
    *                       i)  Number of bytes send on sucess.
    *                       ii) A negative error code on failure
    *                      LORAWAN_STATUS_NOT_INITIALIZED   if system is not initialized with initialize(),
    *                      LORAWAN_STATUS_NO_ACTIVE_SESSIONS if connection is not open,
    *                      LORAWAN_STATUS_WOULD_BLOCK       if another TX is ongoing,
    *                      LORAWAN_STATUS_PORT_INVALID      if trying to send to an invalid port (e.g. to 0)
    *                      LORAWAN_STATUS_PARAMETER_INVALID if NULL data pointer is given or flags are invalid
*/        

int LorawanTP::send_message(uint8_t port, uint8_t payload[], uint16_t length) {
    int8_t retcode=0;
  
    retcode=lorawan.set_device_class(CLASS_C);
    if (retcode!=LORAWAN_STATUS_OK){
        return retcode; 
        }

    retcode=lorawan.send(port, payload, length, MSG_UNCONFIRMED_FLAG); 
    if (retcode < 0) {
        ev_queue.break_dispatch();
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

DownlinkData LorawanTP::receive_message(bool get_data){
   
    uint8_t rx_buffer[51];
    struct DownlinkData data;
    uint64_t decimalValue;
    uint8_t port=0;
    int flags;
   
    if (get_data==false){
    memset(rx_buffer, 0, sizeof(rx_buffer));
    ev_queue.dispatch_forever();
    int8_t retcode = lorawan.receive(rx_buffer, sizeof(rx_buffer), port, flags);
    data.port=port;
    data.retcode=retcode;
    if (retcode<=0){
       ev_queue.break_dispatch();
       return data; 
       }

/** Return the result from hex to decimal
    Port 221 should only be used for setting the time.
    */
    if(port==CLOCK_SYNCH_PORT){
        time_t time_now=time(NULL);
        int bytes_to_buffer=retcode;
        decimalValue=rx_buffer[bytes_to_buffer-1];
        for (int i = 0; i < (retcode-1); i++) {
                int shift_bytes=(8*(bytes_to_buffer-1));
                decimalValue |=(rx_buffer[i]<<shift_bytes);
                bytes_to_buffer--;
            }
        memset(rx_buffer, 0, sizeof(rx_buffer));
        
        data.received_value[0]=decimalValue;
        data.port=0;
        if (decimalValue>time_now){
            set_time(decimalValue+10);
            data.port=port;
            }
      
      } 
    
    if(port==SCHEDULER_PORT){
        int bytes_to_buffer=retcode;
        for (int i = 0; i < retcode; i++) {
            data.received_value[i/2]= rx_buffer[i+1]+(rx_buffer[i]<<8);
            i++;
            }
        memset(rx_buffer, 0, sizeof(rx_buffer));
      }

     if(port==RESET_PORT){
         NVIC_SystemReset();
     }
      else{
        data.port=port;
        int bytes_to_buffer=retcode;
        decimalValue=rx_buffer[bytes_to_buffer-1];
        for (int i = 0; i < (retcode-1); i++) {
                int shift_bytes=(8*(bytes_to_buffer-1));
                decimalValue |=(rx_buffer[i]<<shift_bytes);
                bytes_to_buffer--;
            }
        data.received_value[0]=decimalValue;
        memset(rx_buffer, 0, sizeof(rx_buffer));
      
      }
   
    ev_queue.break_dispatch();
    
   }
  
    return data;
}

/** Put the RF module in sleep mode & lorawan disconnect the current session..
    *
    * @return              It could be one of these:
    *                       i)LORAWAN_STATUS_OK (the statuses are reversed-simplicity reasons) on sucessfull disconnection,
    *                       ii) A negative error code (-1011) on failure to disconeect . */
int LorawanTP::sleep(){
  
    ev_queue.break_dispatch();
    
    _myradio.sleep();
    int retcode=lorawan.disconnect();
    if (retcode==LORAWAN_STATUS_DEVICE_OFF){
        return LORAWAN_STATUS_OK;   
    }
   
    return LORAWAN_STATUS_DEVICE_OFF;
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
            ev_queue.break_dispatch();
            break;
        case DISCONNECTED:
            ev_queue.break_dispatch();
            break;
        case TX_DONE:
            ev_queue.break_dispatch();
            break;
        case TX_TIMEOUT:
            ev_queue.break_dispatch();
            break;
        case TX_ERROR:
            ev_queue.break_dispatch();
            break;
        case TX_CRYPTO_ERROR:
            ev_queue.break_dispatch();
            break;
        case TX_SCHEDULING_ERROR:
            ev_queue.break_dispatch();
            break;
        case RX_DONE:
            ev_queue.break_dispatch();
            break;
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
        case AUTOMATIC_UPLINK_ERROR:
            ev_queue.break_dispatch();
        
            break;
        default:
            MBED_ASSERT("Unknown Event");
    }
}



   

