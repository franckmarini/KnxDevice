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


// File : KnxTpUart.h
// Author : Franck Marini
// Description : Communication with TPUART
// Module dependencies : HardwareSerial, KnxTelegram, KnxComObject

// This library supports both TPUART version 1 and 2
// The Siemens KNX TPUART version 1 datasheet is available at :
// http://www.hqs.sbt.siemens.com/Lowvoltage/gamma_product_data/gamma-b2b/tpuart.pdf
// The Siemens KNX TPUART version 2 datasheet is available at :
// http://www.hqs.sbt.siemens.com/Lowvoltage/gamma_product_data/gamma-b2b/TPUART2_technical-data.pdf

#ifndef KNXTPUART_H
#define KNXTPUART_H

#include "Arduino.h"
#include "HardwareSerial.h"
#include "KnxTelegram.h"
#include "KnxComObject.h"

// !!!!!!!!!!!!!!! FLAG OPTIONS !!!!!!!!!!!!!!!!!
// DEBUG :
// #define KNXTPUART_DEBUG_INFO   // Uncomment to activate info traces
// #define KNXTPUART_DEBUG_ERROR  // Uncomment to activate error traces


// Values returned by the KnxTpUart member functions :
#define KNX_TPUART_OK                            0
#define KNX_TPUART_ERROR                       255
#define KNX_TPUART_ERROR_NOT_INIT_STATE        254
#define KNX_TPUART_ERROR_NULL_EVT_CALLBACK_FCT 253
#define KNX_TPUART_ERROR_NULL_ACK_CALLBACK_FCT 252


// Services to TPUART (hostcontroller -> TPUART) :
#define TPUART_RESET_REQ                     0x01
#define TPUART_STATE_REQ                     0x02
#define TPUART_SET_ADDR_REQ                  0x28
#define TPUART_DATA_START_CONTINUE_REQ       0x80
#define TPUART_DATA_END_REQ                  0x40
#define TPUART_ACTIVATEBUSMON_REQ            0x05
#define TPUART_RX_ACK_SERVICE_ADDRESSED      0x11
#define TPUART_RX_ACK_SERVICE_NOT_ADDRESSED  0x10


// Services from TPUART (TPUART -> hostcontroller) :
// 3 types of data are transmitted from TPUART to the host :
// 1) EIB bus data (transparently transmitted). Format = EIB control field byte + rest of the telegram
// 2) Additional information from the TP-UART. Format = 1 data byte
// 3) Immediate acknowledge services (BUS MONITOR mode only)
#define TPUART_RESET_INDICATION               0x03
#define TPUART_DATA_CONFIRM_SUCCESS           0x8B
#define TPUART_DATA_CONFIRM_FAILED            0x0B
#define TPUART_STATE_INDICATION               0x07
#define TPUART_STATE_INDICATION_MASK          0x07
#define EIB_CONTROL_FIELD_PATTERN_MASK   B11010011
#define EIB_CONTROL_FIELD_VALID_PATTERN  B10010000 // Only Standard Frame Format "10" is handled

// Mask for STATE INDICATION service
#define TPUART_STATE_INDICATION_SLAVE_COLLISION_MASK  0x80
#define TPUART_STATE_INDICATION_RECEIVE_ERROR_MASK    0x40
#define TPUART_STATE_INDICATION_TRANSMIT_ERROR_MASK   0x20
#define TPUART_STATE_INDICATION_PROTOCOL_ERROR_MASK   0x10
#define TPUART_STATE_INDICATION_TEMP_WARNING_MASK     0x08

// Definition of the TP-UART working modes
enum type_KnxTpUartMode { NORMAL,
                          BUS_MONITOR };

// Definition of the TP-UART events sent to the application layer
enum e_KnxTpUartEvent { 
  TPUART_EVENT_RESET = 0,                    // reset received from the TPUART device
  TPUART_EVENT_RECEIVED_EIB_TELEGRAM,        // a new addressed EIB Telegram has been received
  TPUART_EVENT_EIB_TELEGRAM_RECEPTION_ERROR, // a new addressed EIB telegram reception failed
  TPUART_EVENT_STATE_INDICATION              // new TPUART state indication received
 };

// Typedef for events callback function
typedef void (*type_EventCallbackFctPtr) (e_KnxTpUartEvent);

// --- Definitions for the RECEPTION part ----
// RX states
enum e_TpUartRxState {
  RX_RESET = 0,                             // The RX part is awaiting reset execution
  RX_STOPPED,                               // TPUART reset event received, RX activity is stopped
  RX_INIT,                                  // The RX part is awaiting init execution
  RX_IDLE_WAITING_FOR_CTRL_FIELD,           // Idle, no reception ongoing
  RX_EIB_TELEGRAM_RECEPTION_STARTED,        // Telegram reception started (address evaluation not done yet)
  RX_EIB_TELEGRAM_RECEPTION_ADDRESSED,      // Addressed telegram reception ongoing
  RX_EIB_TELEGRAM_RECEPTION_LENGTH_INVALID, // The telegram being received is too long
  RX_EIB_TELEGRAM_RECEPTION_NOT_ADDRESSED   // Tegram reception ongoing but not addressed
};

typedef struct {
  e_TpUartRxState state;        // Current TPUART RX state
  KnxTelegram receivedTelegram; // Where each received telegram is stored (the content is overwritten on each telegram reception)
                                // A TPUART_EVENT_RECEIVED_EIB_TELEGRAM event notifies each content change
  byte addressedComObjectIndex; // Where the index to the targeted com object is stored (the value is overwritten on each telegram reception)
                                // A TPUART_EVENT_RECEIVED_EIB_TELEGRAM event notifies each content change
} type_tpuart_rx;

// --- Definitions for the TRANSMISSION  part ----
// Transmission states
enum e_TpUartTxState {
  TX_RESET = 0,                // The TX part is awaiting reset execution
  TX_STOPPED,                  // TPUART reset event received, TX activity is stopped
  TX_INIT,                     // The TX part is awaiting init execution
  TX_IDLE,                     // Idle, no transmission ongoing
  TX_TELEGRAM_SENDING_ONGOING, // EIB telegram transmission ongoing
  TX_WAITING_ACK               // Telegram transmitted, waiting for ACK/NACK
};

// Acknowledge values following a telegram sending
enum e_TpUartTxAck {
   ACK_RESPONSE = 0,     // TPUART received an ACK following telegram sending
   NACK_RESPONSE,        // TPUART received a NACK following telegram sending (1+3 attempts by default)
   NO_ANSWER_TIMEOUT,    // No answer (Data_Confirm) received from the TPUART
   TPUART_RESET_RESPONSE // TPUART RESET before we get any ACK
};

// Typedef for TX acknowledge callback function
typedef void (*type_AckCallbackFctPtr) (e_TpUartTxAck);

typedef struct tpuart_tx {
  e_TpUartTxState state;            // Current TPUART TX state
  KnxTelegram *sentTelegram;        // Telegram being sent
  type_AckCallbackFctPtr ackFctPtr; // Pointer to callback function for TX ack
  byte nbRemainingBytes;            // Nb of bytes remaining to be transmitted
  byte txByteIndex;                 // Index of the byte to be sent
} type_tpuart_tx;


// --- Typdef for BUS MONITORING mode data ----
typedef struct {
  boolean isEOP;  // True if the data is an End Of Packet
  byte dataByte;  // Last data retrieved on the bus (valid when isEOP is false)
} type_MonitorData;


class KnxTpUart {
    HardwareSerial& _serial;                  // Arduino HW serial port connected to the TPUART
    const word _physicalAddr;                 // Physical address set in the TP-UART
    const type_KnxTpUartMode _mode;           // TpUart working Mode (Normal/Bus Monitor)
    type_tpuart_rx _rx;                       // Reception structure
    type_tpuart_tx _tx;                       // Transmission structure
    type_EventCallbackFctPtr _evtCallbackFct; // Pointer to the EVENTS callback function
    KnxComObject *_comObjectsList;            // Attached list of com objects
    byte _assignedComObjectsNb;               // Nb of assigned com objects
    byte *_orderedIndexTable;                 // Table containing the assigned com objects indexes ordered by increasing @
    byte _stateIndication;                    // Value of the last received state indication
#if defined(KNXTPUART_DEBUG_INFO) || defined(KNXTPUART_DEBUG_ERROR)
    String *_debugStrPtr;
#endif

#ifdef KNXTPUART_DEBUG_INFO
static const char _debugInfoText[];
#endif
#ifdef KNXTPUART_DEBUG_ERROR
static const char _debugErrorText[];
#endif

  public:  
  
  // Constructor / Destructor
    KnxTpUart(HardwareSerial& serial, word physicalAddr, type_KnxTpUartMode _mode);
    ~KnxTpUart();

  // INLINED functions (see definitions later in this file)

    // Set EVENTs callback function
    // return KNX_TPUART_ERROR (255) if the parameter is NULL
    // return KNX_TPUART_ERROR_NOT_INIT_STATE (254) if the TPUART is not in Init state
    // else return OK
    // The function must be called prior to Init() execution
    byte SetEvtCallback(type_EventCallbackFctPtr);

    // Set ACK callback function
    // return KNX_TPUART_ERROR (255) if the parameter is NULL
    // return KNX_TPUART_ERROR_NOT_INIT_STATE (254) if the TPUART is not in Init state
    // else return OK
    // The function must be called prior to Init() execution
    byte SetAckCallback(type_AckCallbackFctPtr);

    // Get the value of the last received State Indication
    // NB : every state indication value change is notified by a "TPUART_EVENT_STATE_INDICATION" event
    byte GetStateIndication(void) const;

    // Get the reference to the telegram received by the TPUART
    // NB : every received telegram content change is notified by a "TPUART_EVENT_RECEIVED_EIB_TELEGRAM" event
    KnxTelegram& GetReceivedTelegram(void);

    // Get the index of the com object targeted by the last received telegram
    byte GetTargetedComObjectIndex(void) const;

    // returns true if there is an activity ongoing (RX/TX) on the TPUART
    // false when there's no activity or when the tpuart is not initialized
    boolean IsActive(void) const;

#if defined(KNXTPUART_DEBUG_INFO) || defined(KNXTPUART_DEBUG_ERROR)
    // Set the string used for debug traces
    void SetDebugString(String *strPtr);
#endif

  // Functions NOT INLINED
    // Reset the Arduino UART port and the TPUART device
    // Return KNX_TPUART_ERROR in case of TPUART reset failure
    byte Reset(void);

    // Attach a list of com objects
    // NB1 : only the objects with "communication" attribute are considered by the TPUART
    // NB2 : In case of objects with identical address, the object with highest index only is considered
    // return KNX_TPUART_ERROR_NOT_INIT_STATE (254) if the TPUART is not in Init state
    // The function must be called prior to Init() execution
    byte AttachComObjectsList(KnxComObject KnxComObjectsList[], byte listSize);

    // Init
    // returns ERROR (255) if the TP-UART is not in INIT state, else returns OK (0)
    // Init must be called after every reset() execution
    byte Init(void);

    // Send a KNX telegram
    // returns ERROR (255) if TX is not available or if the telegram is not valid, else returns OK (0)
    // NB : the source address is forced to TPUART physical address value
    byte SendTelegram(KnxTelegram& sentTelegram);

    // Reception task
    // This function shall be called periodically in order to allow a correct reception of the EIB bus data
    // Assuming the TPUART speed is configured to 19200 baud, a character (8 data + 1 start + 1 parity + 1 stop)
    // is transmitted in 0,58ms.
    // In order not to miss any End Of Packets (i.e. a gap from 2 to 2,5ms), the function shall be called at a max period of 0,5ms.
    // Typical calling period is 400 usec.
    void RXTask(void);

    // Transmission task
    // This function shall be called periodically in order to allow a correct transmission of the EIB bus data
    // Assuming the TP-Uart speed is configured to 19200 baud, a character (8 data + 1 start + 1 parity + 1 stop)
    // is transmitted in 0,58ms.
    // Sending one byte of a telegram consists in transmitting 2 characters (1,16ms)
    // Let's wait around 800us between each telegram piece sending so that the 64byte TX buffer remains almost empty.
    // Typical calling period is 800 usec.
    void TXTask(void);

    // Get Bus monitoring data (BUS MONITORING mode)
    // The function returns true if a new data has been retrieved (data pointer in argument), else false
    // It shall be called periodically (max period of 0,5ms) in order to allow correct data reception
    // Typical calling period is 400 usec.
    boolean GetMonitoringData(type_MonitorData&);

    // DEBUG purpose functions
    void DEBUG_SendResetCommand(void);
    void DEBUG_SendStateReqCommand(void);

  private:

  // Private INLINED functions (see definitions later in this file)
#if defined(KNXTPUART_DEBUG_INFO)
    void DebugInfo(const char[]) const;
#endif
#if defined(KNXTPUART_DEBUG_ERROR)
    void DebugError(const char[]) const;
#endif

  // Private NOT INLINED functions 
    // Check if the target address points to an assigned com object (i.e. the target address equals a com object address)
    // if yes, then update index parameter with the index (in the list) of the targeted com object and return true
    // else return false
    boolean IsAddressAssigned(word addr, byte &index) const;
};


// ----- Definition of the INLINED functions :  ------------

inline byte KnxTpUart::SetEvtCallback(type_EventCallbackFctPtr evtCallbackFct)
{ 
  if (evtCallbackFct == NULL) return KNX_TPUART_ERROR;
  if ((_rx.state!=RX_INIT) || (_tx.state!=TX_INIT)) return KNX_TPUART_ERROR_NOT_INIT_STATE;
  _evtCallbackFct = evtCallbackFct;
  return KNX_TPUART_OK;
}

inline byte KnxTpUart::SetAckCallback(type_AckCallbackFctPtr ackFctPtr)
{
  if (ackFctPtr == NULL) return KNX_TPUART_ERROR;
  if ((_rx.state!=RX_INIT) || (_tx.state!=TX_INIT)) return KNX_TPUART_ERROR_NOT_INIT_STATE;
  _tx.ackFctPtr = ackFctPtr;
  return KNX_TPUART_OK;
}

inline byte KnxTpUart::GetStateIndication(void) const { return _stateIndication; }

inline KnxTelegram& KnxTpUart::GetReceivedTelegram(void)
{ return _rx.receivedTelegram; }


inline byte KnxTpUart::GetTargetedComObjectIndex(void) const
{ return _rx.addressedComObjectIndex; } // return the index of the adress addressed by the received KNX Telegram


inline boolean KnxTpUart::IsActive(void) const
{
  if ( _rx.state > RX_IDLE_WAITING_FOR_CTRL_FIELD) return true; // Rx activity
  if ( _tx.state > TX_IDLE) return true; // Tx activity
  return false;
}


#if defined(KNXTPUART_DEBUG_INFO) || defined(KNXTPUART_DEBUG_ERROR)
inline void KnxTpUart::SetDebugString(String *strPtr)
{
   _debugStrPtr = strPtr;
}
#endif


#if defined(KNXTPUART_DEBUG_INFO)
inline void KnxTpUart::DebugInfo(const char comment[]) const
{
  if (_debugStrPtr != NULL) *_debugStrPtr += String(_debugInfoText) + String(comment);
}
#endif


#if defined(KNXTPUART_DEBUG_ERROR)
inline void KnxTpUart::DebugError(const char comment[]) const
{
  if (_debugStrPtr != NULL) *_debugStrPtr += String(_debugErrorText) + String(comment);
}
#endif

#endif // KNXTPUART_H
