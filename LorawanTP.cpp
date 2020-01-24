/**
  * @file    LorawanTP.cpp
  * @version 0.3.0
  * @author  Rafaella Neofytou
  * @brief   C++ file of the SX1276 driver module. 
  * Handles communication with the thingpilot nodes utilising the SX1276 (or compatible) modem
  */

/** Includes
 */
#include "LorawanTP.h"

/** EventQueue for dispatching events
 */
static EventQueue ev_queue(20*EVENTS_EVENT_SIZE);

#if(OVER_THE_AIR_ACTIVATION)
static uint8_t DEVICE_EUI[8] = { DEV_EUI };
static uint8_t APPLICATION_EUI[8] = { APP_EUI };
static uint8_t APPLICATION_KEY[16] = { APP_KEY };
#endif

#if(!OVER_THE_AIR_ACTIVATION)
static uint8_t net_id = 0x13;
static uint32_t devAddr = DEV_ADD;
static uint8_t nwkSKey[] = { NWKS_KEY };
static uint8_t appSKey[] = { APPS_KEY };
#endif
lorawan_app_callbacks_t cbs;

/** Constructor for the LorawanTP class
 */
LorawanTP::LorawanTP(PinName mosi,PinName miso,PinName sclk,PinName nss,PinName reset,PinName dio0,PinName dio1,PinName dio2,
                     PinName dio3,PinName dio4,PinName dio5,PinName rf_switch_ctl1,PinName rf_switch_ctl2,PinName txctl,
                     PinName rxctl,PinName ant_switch,PinName pwr_amp_ctl,PinName tcxo): 
                     _myradio(mosi,miso,sclk,nss,reset,dio0,dio1,dio2,dio3,dio4,dio5,rf_switch_ctl1,rf_switch_ctl2,txctl,
                     rxctl,ant_switch,pwr_amp_ctl,tcxo), lorawan(_myradio) 
{
          
}

/** Destructor for the LorawanTP class
 */
LorawanTP:: ~LorawanTP()
{
   _myradio.~SX1276_LoRaRadio();
}

/** Join the TTN Network Server.
    *
    * @return              It could be one of these:
    *                       i)  0 sucess.
    *                      ii) A negative error code on failure. */
int LorawanTP::join(const device_class_t device_class)
{   
    lorawan_status_t retcode;
    retcode=lorawan.initialize(&ev_queue);
    if (retcode!=LORAWAN_STATUS_OK)
    {
        return retcode; 
    }

    retcode=lorawan.set_device_class(device_class);
    if (retcode != LORAWAN_STATUS_OK) 
    {
        return retcode; 
    }
    cbs.events = mbed::callback(lora_event_handler);
    lorawan.add_app_callbacks(&cbs);

    //lorawan.set_datarate(0);
    retcode=lorawan.enable_adaptive_datarate();
    if (retcode != LORAWAN_STATUS_OK) 
    {
        return retcode; 
    }

    lorawan_connect_t connect_params;
    #if(OVER_THE_AIR_ACTIVATION)
        connect_params.connect_type = LORAWAN_CONNECTION_OTAA;
        connect_params.connection_u.otaa.dev_eui = DEVICE_EUI;
        connect_params.connection_u.otaa.app_eui = APPLICATION_EUI;
        connect_params.connection_u.otaa.app_key = APPLICATION_KEY;
    #endif
    #if(!OVER_THE_AIR_ACTIVATION)
        connect_params.connect_type = LORAWAN_CONNECTION_ABP;
        connect_params.connection_u.abp.nwk_id = net_id;
        connect_params.connection_u.abp.dev_addr = devAddr;
        connect_params.connection_u.abp.nwk_skey = nwkSKey;
        connect_params.connection_u.abp.app_skey = appSKey;
        
    #endif
    retcode = lorawan.connect(connect_params);
   
    /**TODO: CHECK with OTAA and ABP return codes*/
    if (retcode != LORAWAN_STATUS_OK || retcode !=LORAWAN_STATUS_CONNECT_IN_PROGRESS || retcode!=LORAWAN_STATUS_ALREADY_CONNECTED) 
    {
    //TODO: 
    }
   
/** Dispatch the event,if connected it will stop
    */
    ev_queue.dispatch_forever();
    ev_queue.break_dispatch();

    return LORAWAN_STATUS_OK; 
}

 /** Send a message from the Network Server on a specific port.
    *
    * @param port          The application port number. Port numbers 0 and 220-224 are reserved,
    *                      whereas port numbers from 1 to 219 are valid port numbers.
    *
    * @param payload       A buffer with data from the user or stored in eeproom.
    *
    * @param length        The size of data in bytes.
    *
    *                      A flag is used to determine what type of message is being sent, for example:
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

int LorawanTP::send_message(uint8_t port, uint8_t payload[], uint16_t length) 
{
    int retcode=0;

    retcode=lorawan.send(port, payload, length, MSG_UNCONFIRMED_FLAG); //MSG_CONFIRMED_FLAG 
    if (retcode < LORAWAN_STATUS_OK) 
    {
        debug("Error retcode %d ",retcode);
        ev_queue.break_dispatch();
        return retcode;
    } 
    ev_queue.dispatch_forever();
    return retcode;
}

/** Receives a message from the Network Server on a specific port.
    *
    * @param rx_port       The application port number. Port numbers 0 and 224 are reserved,
    *                      whereas port numbers from 1 to 223 (0x01 to 0xDF) are valid port numbers.
    *                      Anything out of this range is illegal.
    *
    * @param rx_dec_buffer A pointer to buffer where the received data will be stored.
    *
    * @param rx_retcode    The size of data in bytes. If negative is a status code.
    *
    *                      A flag is used to determine what type of message is being received, for example:
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

int LorawanTP::receive_message(uint32_t* rx_dec_buffer, uint8_t* rx_port, int* rx_retcode)  
{
    uint8_t rx_buffer[100]= { 0 };
    uint32_t decimalValue=0;
    uint8_t port=0;
    int retcode = 0;
    int flags;

    ev_queue.dispatch_forever();
    
    memset(rx_buffer, 0, sizeof(rx_buffer));
    debug("Receiving...");
    retcode = lorawan.receive(rx_buffer, sizeof(rx_buffer), port, flags);
    debug("Received...\r\n");
    
    if (retcode<=LORAWAN_STATUS_OK)
    {
       ev_queue.break_dispatch();
       return retcode; 
    }

    if(port==SCHEDULER_PORT)
    {
        int bytes_to_buffer=retcode;
        for (int i = 0; i < retcode; i++)
        {
            rx_dec_buffer[i/2]=rx_buffer[i+1]+(rx_buffer[i]<<8);
            i++;
        }
        memset(rx_buffer, 0, sizeof(rx_buffer));
    }
    
    if(port==RESET_PORT)
    {
        NVIC_SystemReset();
    }

    else
    {
        int bytes_to_buffer=retcode;
        decimalValue=rx_buffer[bytes_to_buffer-1];
        for (int i = 0; i < (retcode-1); i++) 
        {
            int shift_bytes=(8*(bytes_to_buffer-1));
            decimalValue |=(rx_buffer[i]<<shift_bytes);
            bytes_to_buffer--;
        }
        rx_dec_buffer[0]=decimalValue;
        memset(rx_buffer, 0, sizeof(rx_buffer));
    }
   
    ev_queue.break_dispatch();
    *rx_port = port;
    *rx_retcode = retcode;
    return retcode;
}

/** Put the RF module in sleep mode & lorawan disconnect the current session..
    *
    * @return              It could be one of these:
    *                       i)LORAWAN_STATUS_OK (the statuses are reversed-simplicity reasons) on sucessfull disconnection,
    *                       ii) A negative error code (-1011) on failure to disconeect . */
int LorawanTP::sleep()
{
    ev_queue.break_dispatch();
    _myradio.sleep();
    int retcode=lorawan.disconnect();
    if (retcode==LORAWAN_STATUS_DEVICE_OFF)
    {
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
    switch (event) 
    {
        case CONNECTED:
            debug("Connected\r\n");
            ev_queue.break_dispatch();
            break;
        case DISCONNECTED:
            debug("DISCONNECTED\r\n");
            ev_queue.break_dispatch();
            break;
        case TX_DONE:
            debug("Sent Succesfully\r\n");
            ev_queue.break_dispatch();
            break;
        case TX_TIMEOUT:
            debug("TX_TIMEOUT\r\n");
            ev_queue.break_dispatch();
            break;
        case TX_ERROR:
            debug("TX_ERROR\r\n");
            ev_queue.break_dispatch();
            break;
        case TX_CRYPTO_ERROR:
            debug("TX_CRYPTO_ERROR\r\n");
            ev_queue.break_dispatch();
            break;
        case TX_SCHEDULING_ERROR:
            debug("TX_SCHEDULING_ERROR\r\n");
            ev_queue.break_dispatch();
            break;
        case RX_DONE:
            debug("RX_DONE\r\n");
            ev_queue.break_dispatch();
            break;
        case RX_TIMEOUT:
            debug("RX_TIMEOUT\r\n");
        case RX_ERROR:
            debug("ERROR\r\n");
        case JOIN_FAILURE:
            debug("JOIN_FAILURE\r\n");
            ev_queue.break_dispatch();
            break;
        case UPLINK_REQUIRED:
            debug("UPLINK_REQUIRED\r\n");
            ev_queue.break_dispatch();
            break;
        case AUTOMATIC_UPLINK_ERROR:
            debug("AUTOMATIC_UPLINK_ERROR\r\n");
            ev_queue.break_dispatch();
            break;
        default:
            MBED_ASSERT("Unknown Event");
    }
}



   

