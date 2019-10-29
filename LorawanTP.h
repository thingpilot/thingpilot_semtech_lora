/**
  * @file    LorawanTP.h
  * @version 1.0.0
  * @author  
  * @brief   Header file of the SX1276 driver module. 
  * Handles communication to the thingpilot API utilising the SX1272/SX1276 (or compatible) modem
  */

/** Define to prevent recursive inclusion
 */
#pragma once

/** Includes 
 */
#include "mbed.h"
#include <cstdio>
#include "lorawan/LoRaWANInterface.h"
#include "lorawan/system/lorawan_data_structures.h"
#include "events/EventQueue.h"
#include "SX1276_LoRaRadio.h"

#define MSG_RETRIES 3


/** Base class for the LorawanTP
 */ 
class LorawanTP {
    public: 
    enum
    {
        LORA_ERROR                       = -1,
        LORA_OK                          =  0
      
    };

    LorawanTP();
    ~LorawanTP();

   int joinTTN();
   int sleep();
   int send_message(uint8_t port, uint8_t payload[], uint16_t length);
   uint8_t * receive_message();

   private:
   static void lora_event_handler(lorawan_event_t event);
  
};

  