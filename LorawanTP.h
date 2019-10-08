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

/*Needs to be changed for each device*/

static uint8_t LORAWAN_DEV_EUI[] = { 0x00, 0x6E, 0x15, 0x9D, 0x9B, 0x1F, 0x1B, 0x26 };
static uint8_t LORAWAN_APP_EUI[] = { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x02, 0x1A, 0xF3 };
static uint8_t LORAWAN_APP_KEY[] = { 0x66, 0x72, 0x93, 0x0B, 0xE7, 0x5C, 0xD2, 0xFA, 0x49, 0xA8, 0xCB, 0xF7, 0xA4, 0xDD, 0x29, 0x67 };

/** Base class for the LorawanTP
 */ 
class LorawanTP {

    LorawanTP();

    ~LorawanTP();

    public: 
    
    int init();
    void send_message();
    
    
};