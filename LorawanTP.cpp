/**
  * @file    LorawanTP.cpp
  * @version 1.0.0
  * @author  
  * @brief   C++ file of the SX1276 driver module. 
  * Handles communication to the thingpilot API utilising the SX1272/SX1276 (or compatible) modem
  */

/** Includes
 */
#include "LorawanTP.h"
Serial a(PC_10, PC_11);
 static EventQueue ev_queue(10 *EVENTS_EVENT_SIZE);
// static void lora_event_handler(lorawan_event_t event);

// lorawan_app_callbacks_t cbs;

SX1276_LoRaRadio myradio(PA_7,PA_6,PA_5,PB_13,D7,PC_6,PC_7,PC_8,PB_14,PB_15,PC_9,
                            NC,NC,NC,NC,NC,NC,NC);

LoRaWANInterface lorawan(myradio);

uint8_t tx_buffer[30];
uint8_t rx_buffer[30];

int LorawanTP::init(uint8_t LORAWAN_DEV_EUI[],uint8_t LORAWAN_APP_EUI[],uint8_t LORAWAN_APP_KEY[])
{  

    //memset(rx_buffer, 0, sizeof(rx_buffer)); //clear the buffer
    lorawan_status_t retcode;

    a.printf("\r\nStarting.. \r\n");
    retcode=lorawan.initialize(&ev_queue);

    if (retcode!=LORAWAN_STATUS_OK){

            a.printf("\r\nNot initialized. retcode: %d \r\n",retcode);
            a.printf("\r\nNot initialized. retcode: %d \r\n",retcode);
               //wait(5);
               return LORA_ERROR;
            }
    else{

     a.printf("\r\nInitialized retcode: %d \r\n",retcode);
     printf("\r\nInitialized retcode: %d \r\n",retcode);   
    }

    if (lorawan.enable_adaptive_datarate() != LORAWAN_STATUS_OK) {
                a.printf("\r\n enable_adaptive_datarate failed! \r\n");
                return LORA_ERROR;
            }
    else{
    
    printf("Enabled adaptive datarate \r\n");

    }

    lorawan.set_confirmed_msg_retries(3);

    // cbs.events = mbed::callback(lora_event_handler);
    // lorawan.add_app_callbacks(&cbs);

    lorawan_connect_t connect_params;
    connect_params.connect_type = LORAWAN_CONNECTION_OTAA;
            
    connect_params.connection_u.otaa.dev_eui = LORAWAN_DEV_EUI;
    connect_params.connection_u.otaa.app_eui = LORAWAN_APP_EUI;
    connect_params.connection_u.otaa.app_key = LORAWAN_APP_KEY;
    connect_params.connection_u.otaa.nb_trials = 10;


    retcode=lorawan.connect(connect_params);

    if (retcode == LORAWAN_STATUS_OK || retcode ==LORAWAN_STATUS_CONNECT_IN_PROGRESS) {
            a.printf("Connected, retcode: %d \r\n",retcode);   
            }
    else{
    
     printf("FAILED to connect! retcode: %d \r\n",retcode);
                return LORA_ERROR;
    

    }
    ev_queue.dispatch_forever();

    return LORA_OK;         
}


 int16_t LorawanTP::send_message(uint8_t port, const uint8_t *data, uint16_t length) {
            int16_t retcode;
            uint8_t payload[9];
            int sensor_value_t =10;
            
            payload[0]=0;
            payload[1]= 10>>8 & 0xFF; // two byte number
            payload[2]=10 & 0xFF;
            payload[3]=10 & 0xFF;
            payload[4]= 10>>16 & 0xFF; // two byte number
            payload[5]= 10>>8 & 0xFF;
            payload[6]=10 & 0xFF;
            payload[7]=10 & 0xFF;
            payload[8]=36 & 0xFF;

    
            retcode=lorawan.send(1, payload, sizeof(payload), MSG_UNCONFIRMED_FLAG);  
            printf("\r\n Send %d bytes\r\n",retcode);

            if (retcode < 0) {
        retcode == LORAWAN_STATUS_WOULD_BLOCK ? printf("send - WOULD BLOCK\r\n")
                : printf("\r\n send() - Error code %d \r\n", retcode);
        return LORA_ERROR;
            }
       return LORA_OK;
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
     int16_t LorawanTP::receive_message()
{
    uint8_t port;
    int flags;
    int16_t retcode = lorawan.receive(rx_buffer, sizeof(rx_buffer), port, flags);

    if (retcode < 0) {
        a.printf("\r\n receive() - Error code %d \r\n", retcode);
        return retcode;
    }

    printf(" RX Data on port %u (%d bytes): ", port, retcode);
    for (uint8_t i = 0; i < retcode; i++) {
        a.printf("%02x ", rx_buffer[i]);
    }
    printf("\r\n");
    
    memset(rx_buffer, 0, sizeof(rx_buffer));
    return LORA_OK;

}


// void LorawanTP::lora_event_handler(lorawan_event_t event)
// {
//     switch (event) {
//         case CONNECTED:
//             printf("\r\n Connection - Successful \r\n");
//             if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
//                 uint8_t payload[];

//                 LorawanTP::send_message(1, payload, sizeof(payload));
//             // } else {
//             //     ev_queue.call_every(TX_TIMER, send_message);
//             // }

//             break;
//         case DISCONNECTED:
//             ev_queue.break_dispatch();
//             printf("\r\n Disconnected Successfully \r\n");
//             break;
//         case TX_DONE:
//             printf("\r\n Message Sent to Network Server \r\n");
//             if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
//                 send_message();
//             }
//             break;
//         case TX_TIMEOUT:
//         case TX_ERROR:
//         case TX_CRYPTO_ERROR:
//         case TX_SCHEDULING_ERROR:
//             printf("\r\n Transmission Error - EventCode = %d \r\n", event);
//             // try again
//             if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
//                 send_message();
//             }
//             break;
//         case RX_DONE:
//             printf("\r\n Received message from Network Server \r\n");
//             receive_message();
//             break;
//         case RX_TIMEOUT:
//         case RX_ERROR:
//             printf("\r\n Error in reception - Code = %d \r\n", event);
//             break;
//         case JOIN_FAILURE:
//             printf("\r\n OTAA Failed - Check Keys \r\n");
//             break;
//         case UPLINK_REQUIRED:
//             printf("\r\n Uplink required by NS \r\n");
//             if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
//                 send_message();
//             }
//             break;
//         default:
//             MBED_ASSERT("Unknown Event");
//     }
// }



   
    //    static void lora_event_handler(lorawan_event_t event)
    //   {
    //         switch (event) {
    //     case CONNECTED:
    //         printf("\r\n Connection - Successful \r\n");
    //             send_message();

    //             myradio.sleep();
    //             lorawan.disconnect();
           
    //         break;
    //     case DISCONNECTED:
    //         ev_queue.break_dispatch();
    //         printf("\r\n Disconnected Successfully \r\n");
    //         break;
    //     case TX_DONE:
    //     case TX_TIMEOUT:
    //     case TX_ERROR:
    //     case TX_CRYPTO_ERROR:
    //     case TX_SCHEDULING_ERROR:
        
    //         break;
    //     case JOIN_FAILURE:
    //         printf("\r\n OTAA Failed - Check Keys \r\n");
    //         break;
    //     default:
    //         MBED_ASSERT("Unknown Event");
    // }


    //   }

