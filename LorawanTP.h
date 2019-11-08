/**
  * @file    LorawanTP.h
  * @version 1.1.0
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
#include "lorawan/LoRaWANInterface.h"
#include "lorawan/system/lorawan_data_structures.h"
#include "events/EventQueue.h"
#include "lorawan/LoRaRadio.h"
#include "SX1276_LoRaRadio.h"
#include "board.h"

/** Struct used to store parameters that enable the storage,
     *  modification and deletion of entries within a 'file'
     */
	

    struct DownlinkData
    {
        uint16_t received_value[100];
        uint8_t port;
    } ;
  
/** Base class for the LorawanTP
 */ 
class LorawanTP {

    public: 
    
    LorawanTP();
    ~LorawanTP();
    
    int sleep();
    int join();
    
    int send_message(uint8_t port, uint8_t payload[], uint16_t length);
    DownlinkData receive_message();
    
    private:
    static void lora_event_handler(lorawan_event_t event);
    
  
};

  