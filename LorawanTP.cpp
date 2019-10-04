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


 Serial pc(PC_10, PC_11);

static uint8_t LORAWAN_DEV_EUI[] = { 0x00, 0x6E, 0x15, 0x9D, 0x9B, 0x1F, 0x1B, 0x26 };
static uint8_t LORAWAN_APP_EUI[] = { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x02, 0x1A, 0xF3 };
static uint8_t LORAWAN_APP_KEY[] = { 0x66, 0x72, 0x93, 0x0B, 0xE7, 0x5C, 0xD2, 0xFA, 0x49, 0xA8, 0xCB, 0xF7, 0xA4, 0xDD, 0x29, 0x67 };

static EventQueue ev_queue(10 *EVENTS_EVENT_SIZE);
static void lora_event_handler(lorawan_event_t event);


lorawan_app_callbacks_t cbs;

SX1276_LoRaRadio myradio(PA_7,PA_6,PA_5,PB_13,D7,PC_6,PC_7,PC_8,PB_14,PB_15,PC_9,
                            NC,NC,NC,NC,NC,NC,NC);

LoRaWANInterface lorawan(myradio);

int main()
{  
    pc.baud(9600);
    lorawan_status_t retcode;

    pc.printf("\r\nStarting.. \r\n");
    retcode=lorawan.initialize(&ev_queue);

    if (retcode!=LORAWAN_STATUS_OK){

            pc.printf("\r\nNot initialized. retcode: %d \r\n",retcode);
            printf("\r\nNot initialized. retcode: %d \r\n",retcode);
               //wait(5);
               return -1;
            }
    else{

     pc.printf("\r\nInitialized retcode: %d \r\n",retcode);
     printf("\r\nInitialized retcode: %d \r\n",retcode);   
    }

    if (lorawan.enable_adaptive_datarate() != LORAWAN_STATUS_OK) {
                pc.printf("\r\n enable_adaptive_datarate failed! \r\n");
                return -1;
            }
    else{
    
    pc.printf("Enabled adaptive datarate \r\n");

    }

    lorawan.set_confirmed_msg_retries(3);

    cbs.events = mbed::callback(lora_event_handler);
    lorawan.add_app_callbacks(&cbs);

    lorawan_connect_t connect_params;
    connect_params.connect_type = LORAWAN_CONNECTION_OTAA;
            
    connect_params.connection_u.otaa.dev_eui = LORAWAN_DEV_EUI;
    connect_params.connection_u.otaa.app_eui = LORAWAN_APP_EUI;
    connect_params.connection_u.otaa.app_key = LORAWAN_APP_KEY;
    connect_params.connection_u.otaa.nb_trials = 10;


    retcode=lorawan.connect(connect_params);

    if (retcode == LORAWAN_STATUS_OK || retcode ==LORAWAN_STATUS_CONNECT_IN_PROGRESS) {
            pc.printf("Connected, retcode: %d \r\n",retcode);   
            }
    else{
    
     pc.printf("FAILED to connect! retcode: %d \r\n",retcode);
                return -1;
    

    }

    ev_queue.dispatch_forever();

    return 0;
  
           
}


  static void send_message() {
            int16_t retcode;
            uint8_t payload[]={0,1,2,3,4,5,6,7,8};
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
            pc.printf("\r\n Send %d bytes\r\n",retcode);

            if (retcode < 0) {
        retcode == LORAWAN_STATUS_WOULD_BLOCK ? pc.printf("send - WOULD BLOCK\r\n")
                : pc.printf("\r\n send() - Error code %d \r\n", retcode);
        return;
            }

         
        //    pc.printf("\r\n Disconnected Successfully \r\n");
        //    ev_queue.break_dispatch();

       
       }

   
       static void lora_event_handler(lorawan_event_t event)
      {
            switch (event) {
        case CONNECTED:
            pc.printf("\r\n Connection - Successful \r\n");
                send_message();
           
            break;
        case DISCONNECTED:
            ev_queue.break_dispatch();
            pc.printf("\r\n Disconnected Successfully \r\n");
            break;
        case TX_DONE:
        case TX_TIMEOUT:
        case TX_ERROR:
        case TX_CRYPTO_ERROR:
        case TX_SCHEDULING_ERROR:
        
            break;
        case JOIN_FAILURE:
            printf("\r\n OTAA Failed - Check Keys \r\n");
            break;
        default:
            MBED_ASSERT("Unknown Event");
    }


      }

