//    This file is part of Arduino Knx Bus Device library.

//    The Arduino Knx Bus Device library allows to turn Arduino into "self-made" KNX bus device.
//    Copyright (C) 2014 2015 2016 Franck MARINI (fm@liwan.fr)

//    The Arduino Knx Bus Device library is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

// File : KnxDevice.h
// Author : Franck Marini
// Description : KnxDevice Abstraction Layer
// Module dependencies : HardwareSerial, KnxTelegram, KnxComObject, KnxTpUart, ActionRingBuffer

#ifndef KNXDEVICE_H
#define KNXDEVICE_H

#include "Arduino.h"
#include "KnxTelegram.h"
#include "KnxComObject.h"
#include "ActionRingBuffer.h"
#include "KnxTpUart.h"

// !!!!!!!!!!!!!!! FLAG OPTIONS !!!!!!!!!!!!!!!!!
// DEBUG :
// #define KNXDEVICE_DEBUG_INFO   // Uncomment to activate info traces

// Values returned by the KnxDevice member functions :
enum e_KnxDeviceStatus {
  KNX_DEVICE_OK = 0,
  KNX_DEVICE_NOT_IMPLEMENTED = 254,
  KNX_DEVICE_ERROR = 255
};

// Macro functions for conversion of physical and 2/3 level group addresses
inline word P_ADDR(byte area, byte line, byte busdevice)
{ return (word) ( ((area&0xF)<<12) + ((line&0xF)<<8) + busdevice ); }

inline word G_ADDR(byte maingrp, byte midgrp, byte subgrp)
{ return (word) ( ((maingrp&0x1F)<<11) + ((midgrp&0x7)<<8) + subgrp ); }

inline word G_ADDR(byte maingrp, byte subgrp)
{ return (word) ( ((maingrp&0x1F)<<11) + subgrp ); }

#define ACTIONS_QUEUE_SIZE 16

// KnxDevice internal state
enum e_KnxDeviceState {
  INIT,
  IDLE,
  TX_ONGOING,
};

// Action types
enum e_KnxDeviceTxActionType {
  EIB_READ_REQUEST,
  EIB_WRITE_REQUEST,
  EIB_RESPONSE_REQUEST
};

struct struct_tx_action{
  e_KnxDeviceTxActionType command; // Action type to be performed
  byte index; // Index of the involved ComObject
  union { // Value
    // Field used in case of short value (value width <= 1 byte)
    struct {
      byte byteValue;
      byte notUsed;
    };
    byte *valuePtr; // Field used in case of long value (width > 1 byte), space is allocated dynamically
  };
};// type_tx_action;

typedef struct struct_tx_action type_tx_action;


// Callback function to catch and treat KNX events
// The definition shall be provided by the end-user
extern void knxEvents(byte);


// --------------- Definition of the functions for DPT translation --------------------
// Functions to convert a DPT format to a standard C type
// NB : only the usual DPT formats are supported (U16, V16, U32, V32, F16 and F32)
template <typename T> e_KnxDeviceStatus ConvertFromDpt(const byte dpt[], T& result, byte dptFormat);

// Functions to convert a standard C type to a DPT format
// NB : only the usual DPT formats are supported (U16, V16, U32, V32, F16 and F32)
template <typename T> e_KnxDeviceStatus ConvertToDpt(T value, byte dpt[], byte dptFormat);


class KnxDevice {
    static KnxComObject _comObjectsList[];          // List of Com Objects attached to the KNX Device
                                                    // The definition shall be provided by the end-user
    static const byte _comObjectsNb;                // Nb of attached Com Objects
                                                    // The value shall be provided by the end-user
    e_KnxDeviceState _state;                        // Current KnxDevice state
    KnxTpUart *_tpuart;                             // TPUART associated to the KNX Device
    ActionRingBuffer<type_tx_action, ACTIONS_QUEUE_SIZE> _txActionList; // Queue of transmit actions to be performed
    boolean _initCompleted;                         // True when all the Com Object with Init attr have been initialized
    byte _initIndex;                                // Index to the last initiated object
    word _lastInitTimeMillis;                       // Time (in msec) of the last init (read) request on the bus
    word _lastRXTimeMicros;                         // Time (in msec) of the last Tpuart Rx activity;
    word _lastTXTimeMicros;                         // Time (in msec) of the last Tpuart Tx activity;
    KnxTelegram _txTelegram;                        // Telegram object used for telegrams sending
    KnxTelegram *_rxTelegram;                       // Reference to the telegram received by the TPUART
#if defined(KNXDEVICE_DEBUG_INFO)
    byte _nbOfInits;                                // Nb of Initialized Com Objects
    String *_debugStrPtr;
    static const char _debugInfoText[];
#endif

  // Constructor, Destructor
    KnxDevice();  // private constructor (singleton design pattern)
    ~KnxDevice() {}  // private destructor (singleton design pattern)
    KnxDevice (const KnxDevice&); // private copy constructor (singleton design pattern) 

  public:
    static KnxDevice Knx; // unique KnxDevice instance (singleton design pattern)

    // Start the KNX Device
    // return KNX_DEVICE_ERROR (255) if begin() failed
    // else return KNX_DEVICE_OK
    e_KnxDeviceStatus begin(HardwareSerial& serial, word physicalAddr);

    // Stop the KNX Device
    void end();

    // KNX device execution task
    // This function shall be called in the "loop()" Arduino function
    void task(void);

    // Quick method to read a short (<=1 byte) com object
    // NB : The returned value will be hazardous in case of use with long objects
    byte read(byte objectIndex);  

    // Read an usual format com object
    // Supported DPT formats are short com object, U16, V16, U32, V32, F16 and F32
    template <typename T>  e_KnxDeviceStatus read(byte objectIndex, T& returnedValue);

    // Read any type of com object (DPT value provided as is)
    e_KnxDeviceStatus read(byte objectIndex, byte returnedValue[]);

    // Update com object functions :
    // For all the update functions, the com object value is updated locally
    // and a telegram is sent on the EIB bus if the object has both COMMUNICATION & TRANSMIT attributes set

    // Update an usual format com object
    // Supported DPT types are short com object, U16, V16, U32, V32, F16 and F32
    template <typename T>  e_KnxDeviceStatus write(byte objectIndex, T value);

    // Update any type of com object (rough DPT value shall be provided)
    e_KnxDeviceStatus write(byte objectIndex, byte valuePtr[]);
    

    // Com Object EIB Bus Update request
    // Request the local object to be updated with the value from the bus
    // NB : the function is asynchroneous, the update completion is notified by the knxEvents() callback
    void update(byte objectIndex);

    // The function returns true if there is rx/tx activity ongoing, else false
    boolean isActive(void) const;

    // Inline Debug function (definition later in this file)
    // Set the string used for debug traces
#if defined(KNXDEVICE_DEBUG_INFO)
    void SetDebugString(String *strPtr);
#endif

  private:
    // Static GetTpUartEvents() function called by the KnxTpUart layer (callback)
    static void GetTpUartEvents(e_KnxTpUartEvent event);

    // Static TxTelegramAck() function called by the KnxTpUart layer (callback)
    static void TxTelegramAck(e_TpUartTxAck);

#if defined(KNXDEVICE_DEBUG_INFO)
    // Inline Debug function (definition later in this file)
    void DebugInfo(const char[]) const;
#endif
};

#if defined(KNXDEVICE_DEBUG_INFO)
// Set the string used for debug traces
inline void KnxDevice::SetDebugString(String *strPtr) {_debugStrPtr = strPtr;}
#endif


#if defined(KNXDEVICE_DEBUG_INFO)
inline void KnxDevice::DebugInfo(const char comment[]) const
{
	if (_debugStrPtr != NULL) *_debugStrPtr += String(_debugInfoText) + String(comment);
}
#endif

// Reference to the KnxDevice unique instance
extern KnxDevice& Knx;

#endif // KNXDEVICE_H
