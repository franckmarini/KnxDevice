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


// File : KnxTpUart.cpp
// Author : Franck Marini
// Description : Communication with TPUART
// Module dependencies : HardwareSerial, KnxTelegram, KnxComObject

#include "KnxTpUart.h"

static inline word TimeDeltaWord(word now, word before) { return (word)(now - before); }

#ifdef KNXTPUART_DEBUG_INFO
const char KnxTpUart::_debugInfoText[] = "KNXTPUART INFO: ";
#endif

#ifdef KNXTPUART_DEBUG_ERROR
const char KnxTpUart::_debugErrorText[] = "KNXTPUART ERROR: ";
#endif


// Constructor
KnxTpUart::KnxTpUart(HardwareSerial& serial, word physicalAddr, type_KnxTpUartMode mode)
: _serial(serial), _physicalAddr(physicalAddr), _mode(mode)
{
  _rx.state = RX_RESET;
  _rx.addressedComObjectIndex = 0;
  _tx.state = TX_RESET;
  _tx.sentTelegram = NULL;
  _tx.ackFctPtr = NULL;
  _tx.nbRemainingBytes = 0;
  _tx.txByteIndex = 0;
  _stateIndication = 0;
  _evtCallbackFct = NULL;
  _comObjectsList = NULL;
  _assignedComObjectsNb = 0;
  _orderedIndexTable = NULL;
  _stateIndication = 0;
#if defined(KNXTPUART_DEBUG_INFO) || defined(KNXTPUART_DEBUG_ERROR)
  _debugStrPtr = NULL;
#endif
}


// Destructor
KnxTpUart::~KnxTpUart()
{
  if (_orderedIndexTable) free(_orderedIndexTable);
  // close the serial communication if opened
  if ( (_rx.state > RX_RESET) || (_tx.state > TX_RESET) ) 
  {
    _serial.end();
#if defined(KNXTPUART_DEBUG_INFO)
    DebugInfo("Destructor: connection closed, byebye\n");
#endif
  }
#if defined(KNXTPUART_DEBUG_INFO)
  else DebugInfo("Desctructor: byebye\n");
#endif
}


// Reset the Arduino UART port and the TPUART device
// Return KNX_TPUART_ERROR in case of TPUART Reset failure
byte KnxTpUart::Reset(void)
{
word startTime, nowTime;
byte attempts = 10;

  if ( (_rx.state > RX_RESET) || (_tx.state > TX_RESET) ) 
  { // HOT RESET case
    _serial.end(); // stop the serial communication before restarting it
    _rx.state = RX_RESET; _tx.state = TX_RESET;
  }

  // CONFIGURATION OF THE ARDUINO USART WITH CORRECT FRAME FORMAT (19200, 8 bits, parity even, 1 stop bit)
  _serial.begin(19200,SERIAL_8E1);
  //_serial.begin(19200);
  //UCSR1C = UCSR1C | B00100000; // Even Parity
  
  while(attempts--)
  { // we send a RESET REQUEST and wait for the reset indication answer
    // the sequence is repeated every sec as long as we do not get the reset indication 
    _serial.write(TPUART_RESET_REQ); // send RESET REQUEST

    for (nowTime = startTime = (word) millis() ; TimeDeltaWord(nowTime,startTime) < 1000 /* 1 sec */ ; nowTime = (word)millis())
    {
      if (_serial.available() > 0) 
      {
        if (_serial.read() == TPUART_RESET_INDICATION)
        {
          _rx.state = RX_INIT; _tx.state = TX_INIT;
#if defined(KNXTPUART_DEBUG_INFO)
          DebugInfo("Reset successful\n");
#endif
          return KNX_TPUART_OK;
        }
      }
    } // 1 sec ellapsed
  } // while(attempts--)
  _serial.end();
#if defined(KNXTPUART_DEBUG_ERROR)
  DebugError("Reset failed, no answer from TPUART device\n");
#endif
  return KNX_TPUART_ERROR;
}


// Attach a list of com objects
// NB1 : only the objects with "communication" attribute are considered by the TPUART
// NB2 : In case of objects with identical address, the object with highest index only is considered
// return KNX_TPUART_ERROR_NOT_INIT_STATE (254) if the TPUART is not in Init state
// The function must be called prior to Init() execution
byte KnxTpUart::AttachComObjectsList(KnxComObject comObjectsList[], byte listSize)
{
#define IS_COM(index) (comObjectsList[index].GetIndicator() & KNX_COM_OBJ_C_INDICATOR)
#define ADDR(index) (comObjectsList[index].GetAddr())

  if ((_rx.state!=RX_INIT) || (_tx.state!=TX_INIT)) return KNX_TPUART_ERROR_NOT_INIT_STATE;

  if (_orderedIndexTable)
  {  // a list is already attached, we detach it
    free(_orderedIndexTable);
    _orderedIndexTable = NULL;
    _comObjectsList = NULL;
    _assignedComObjectsNb = 0;
  }
  if ((!comObjectsList) || (!listSize))
  {
#if defined(KNXTPUART_DEBUG_INFO)
    DebugInfo("AttachComObjectsList : warning : empty object list!\n");
#endif
    return  KNX_TPUART_OK;
  }
  // Count all the com objects with communication indicator
  for (byte i=0; i < listSize ; i++) if (IS_COM(i)) _assignedComObjectsNb++;
  if (!_assignedComObjectsNb)
  {
#if defined(KNXTPUART_DEBUG_INFO)
    DebugInfo("AttachComObjectsList : warning : no object with com attribute in the list!\n");
#endif
    return  KNX_TPUART_OK;    
  }
  // Deduct the duplicate addresses
  for (byte i=0; i < listSize ; i++)
  {
    if (!IS_COM(i)) continue;
    for (byte j=0; j < listSize ; j++)
    {
      if ( (i!=j) && (ADDR(j) == ADDR(i)) && (IS_COM(j)) )
      { // duplicate address found
        if (j<i) break; // duplicate address already treated
        else 
        {
          _assignedComObjectsNb--;
#if defined(KNXTPUART_DEBUG_INFO)
          DebugInfo("AttachComObjectsList : warning : duplicate address found!\n");
#endif
        }
      }
    }
  }
  _comObjectsList = comObjectsList;
  // Creation of the ordered index table
  _orderedIndexTable = (byte*) malloc(_assignedComObjectsNb);
  word minMin = 0x0000;   // minimum min value searched  
  word foundMin = 0xFFFF; // min value found so far
  for (byte i=0; i < _assignedComObjectsNb; i++)
  {
    for (byte j=0; j < listSize ; j++) 
    {
      if ( (IS_COM(j)) && (ADDR(j)>=minMin) && (ADDR(j)<=foundMin) )
      {
        foundMin = ADDR(j);
        _orderedIndexTable[i] = j;
      }
    }
    minMin = foundMin + 1;
    foundMin = 0xFFFF;
  }
#if defined(KNXTPUART_DEBUG_INFO)
  DebugInfo("AttachComObjectsList successful\n");
#endif
  return KNX_TPUART_OK;
}


// Init
// returns ERROR (255) if the TP-UART is not in INIT state, else returns OK (0)
// Init must be called after every reset() execution
byte KnxTpUart::Init(void)
{
  byte tpuartCmd[3];

  if ((_rx.state!=RX_INIT) || (_tx.state!=TX_INIT)) return KNX_TPUART_ERROR_NOT_INIT_STATE;

  // BUS MONITORING MODE in case it is selected
  if (_mode == BUS_MONITOR)
  {
    _serial.write(TPUART_ACTIVATEBUSMON_REQ); // Send bus monitoring activation request
#if defined(KNXTPUART_DEBUG_INFO)
    DebugInfo("Init : Monitoring mode started\n");
#endif
  }
  else // NORMAL mode by default
  {
#if defined(KNXTPUART_DEBUG_INFO)
    if (_comObjectsList == NULL)  DebugInfo("Init : warning : empty object list!\n");
#endif
    if (_evtCallbackFct == NULL) return KNX_TPUART_ERROR_NULL_EVT_CALLBACK_FCT;
    if (_tx.ackFctPtr == NULL) return KNX_TPUART_ERROR_NULL_ACK_CALLBACK_FCT;

    // Set Physical address. This allows to activate address evaluation by the TPUART
    tpuartCmd[0] = TPUART_SET_ADDR_REQ;
    tpuartCmd[1] = (byte)(_physicalAddr>>8);
    tpuartCmd[2] = (byte)_physicalAddr;
    _serial.write(tpuartCmd,3);
  
    // Call U_State.request-Service in order to have the field _stateIndication up-to-date
    _serial.write(TPUART_STATE_REQ);

    _rx.state = RX_IDLE_WAITING_FOR_CTRL_FIELD;
    _tx.state = TX_IDLE;
#if defined(KNXTPUART_DEBUG_INFO)
    DebugInfo("Init : Normal mode started\n");
#endif
  }
  return KNX_TPUART_OK;
}


// Send a KNX telegram
// returns ERROR (255) if TX is not available, or if the telegram is not valid, else returns OK (0)
// NB : the source address is forced to TPUART physical address value
byte KnxTpUart::SendTelegram(KnxTelegram& sentTelegram)
{
  if (_tx.state != TX_IDLE) return KNX_TPUART_ERROR; // TX not initialized or busy
  
  if (sentTelegram.GetSourceAddress() != _physicalAddr) // Check that source addr equals TPUART physical addr
  { // if not, let's force source addr to the correct value
    sentTelegram.SetSourceAddress(_physicalAddr);
    sentTelegram.UpdateChecksum();
  }
  _tx.sentTelegram = &sentTelegram;
  _tx.nbRemainingBytes = sentTelegram.GetTelegramLength();
  _tx.txByteIndex = 0; // Set index to 0
  _tx.state = TX_TELEGRAM_SENDING_ONGOING;
  return KNX_TPUART_OK;
}


// Reception task
// This function shall be called periodically in order to allow a correct reception of the EIB bus data
// Assuming the TPUART speed is configured to 19200 baud, a character (8 data + 1 start + 1 parity + 1 stop)
// is transmitted in 0,58ms.
// In order not to miss any End Of Packets (i.e. a gap from 2 to 2,5ms), the function shall be called at a max period of 0,5ms.
// Typical calling period is 400 usec.
void KnxTpUart::RXTask(void)
{
byte incomingByte;
word nowTime;
static byte readBytesNb; // Nb of read bytes during an EIB telegram reception
static KnxTelegram telegram; // telegram being received
static byte addressedComObjectIndex; // index of the com object targeted by the received telegram
static word lastByteRxTimeMicrosec;

// === STEP 1 : Check EOP in case a Telegram is being received ===
  if (_rx.state >= RX_EIB_TELEGRAM_RECEPTION_STARTED)
  { // a telegram reception is ongoing
    nowTime = (word) micros(); // word cast because a 65ms looping counter is long enough
    if(TimeDeltaWord(nowTime,lastByteRxTimeMicrosec) > 2000 /* 2 ms */ )
    { // EOP detected, the telegram reception is completed

      switch (_rx.state)
      {
        case RX_EIB_TELEGRAM_RECEPTION_STARTED : // we are not supposed to get EOP now, the telegram is incomplete
        case RX_EIB_TELEGRAM_RECEPTION_LENGTH_INVALID :
          _evtCallbackFct(TPUART_EVENT_EIB_TELEGRAM_RECEPTION_ERROR); // Notify telegram reception error
          break;

        case RX_EIB_TELEGRAM_RECEPTION_ADDRESSED :
          if (telegram.IsChecksumCorrect())
          { // checksum correct, let's update the _rx struct with the received telegram and correct index
        	telegram.Copy(_rx.receivedTelegram);
            _rx.addressedComObjectIndex  = addressedComObjectIndex;
            _evtCallbackFct(TPUART_EVENT_RECEIVED_EIB_TELEGRAM); // Notify the new received telegram
          }
          else
          {  // checksum incorrect, notify error
            _evtCallbackFct(TPUART_EVENT_EIB_TELEGRAM_RECEPTION_ERROR); // Notify telegram reception error
          }
          break;

        // case RX_EIB_TELEGRAM_RECEPTION_NOT_ADDRESSED : break; // nothing to do!
      
        default : break; 
      } // end of switch

      // we move state back to RX IDLE in any case
      _rx.state = RX_IDLE_WAITING_FOR_CTRL_FIELD;
    } // end EOP detected
  }
  
// === STEP 2 : Get New RX Data ===
  if (_serial.available() > 0) 
  {
    incomingByte = (byte)(_serial.read());
    lastByteRxTimeMicrosec = (word)micros();
	
    switch (_rx.state)
    {
      case RX_IDLE_WAITING_FOR_CTRL_FIELD:
          // CASE OF EIB MESSAGE
          if ((incomingByte & EIB_CONTROL_FIELD_PATTERN_MASK) == EIB_CONTROL_FIELD_VALID_PATTERN)
          {
            _rx.state = RX_EIB_TELEGRAM_RECEPTION_STARTED; 
            readBytesNb = 1; telegram.WriteRawByte(incomingByte,0);
          }
          // CASE OF TPUART_DATA_CONFIRM_SUCCESS NOTIFICATION
          else if (incomingByte == TPUART_DATA_CONFIRM_SUCCESS) 
          {
            if (_tx.state == TX_WAITING_ACK)
            {
              _tx.ackFctPtr(ACK_RESPONSE);
              _tx.state = TX_IDLE;
            }
#if defined(KNXTPUART_DEBUG_ERROR)
            else DebugError("Rx: unexpected TPUART_DATA_CONFIRM_SUCCESS received!\n");
#endif
          }
          // CASE OF TPUART_RESET NOTIFICATION
          else if (incomingByte == TPUART_RESET_INDICATION)
          {
        
            if ( (_tx.state == TX_TELEGRAM_SENDING_ONGOING ) || (_tx.state == TX_WAITING_ACK ) )
            { // response to the TP UART transmission
              _tx.ackFctPtr(TPUART_RESET_RESPONSE);
            }
           _tx.state = TX_STOPPED;
           _rx.state = RX_STOPPED;
           _evtCallbackFct(TPUART_EVENT_RESET); // Notify RESET
           return;
          }
          // CASE OF STATE_INDICATION RESPONSE
          else if ((incomingByte & TPUART_STATE_INDICATION_MASK) == TPUART_STATE_INDICATION)
          {
            _evtCallbackFct(TPUART_EVENT_STATE_INDICATION); // Notify STATE INDICATION
            _stateIndication = incomingByte;
#if defined(KNXTPUART_DEBUG_INFO)
            DebugInfo("Rx: State Indication Received\n");
#endif
          }
          // CASE OF TPUART_DATA_CONFIRM_FAILED NOTIFICATION
          else if (incomingByte == TPUART_DATA_CONFIRM_FAILED) 
          {
            // NACK following Telegram transmission
            if (_tx.state == TX_WAITING_ACK)
            {
              _tx.ackFctPtr(NACK_RESPONSE);
              _tx.state = TX_IDLE; 
            }
#if defined(KNXTPUART_DEBUG_ERROR)
            else DebugError("Rx: unexpected TPUART_DATA_CONFIRM_FAILED received!\n");
#endif
          }
#if defined(KNXTPUART_DEBUG_ERROR)
          // UNKNOWN CONTROL FIELD RECEIVED
          else if (incomingByte)
            DebugError("Rx: Unknown Control Field received\n");
#endif
          // else ignore "0" value sent on Reset by TPUART prior to TPUART_RESET_INDICATION
          break;

      case RX_EIB_TELEGRAM_RECEPTION_STARTED :
          telegram.WriteRawByte(incomingByte,readBytesNb);
          readBytesNb++;

          if (readBytesNb==3) 
          {  // We have just received the source address
             // we check whether the received EIB telegram is coming from us (i.e. telegram is sent by the TPUART itself)
            if ( telegram.GetSourceAddress() == _physicalAddr )
            { // the message is coming from us, we consider it as not addressed and we don't send any ACK service
              _rx.state = RX_EIB_TELEGRAM_RECEPTION_NOT_ADDRESSED;
            }
          }
          else if (readBytesNb==6) // We have just read the routing field containing the address type and the payload length
          { // We check if the message is addressed to us in order to send the appropriate acknowledge
            if(IsAddressAssigned(telegram.GetTargetAddress(), addressedComObjectIndex))
            { // Message addressed to us
              _rx.state = RX_EIB_TELEGRAM_RECEPTION_ADDRESSED;
              //sent the correct ACK service now
              // the ACK info must be sent latest 1,7 ms after receiving the address type octet of an addressed frame
              _serial.write(TPUART_RX_ACK_SERVICE_ADDRESSED);
            }
            else
            { // Message NOT addressed to us
              _rx.state = RX_EIB_TELEGRAM_RECEPTION_NOT_ADDRESSED;
              //sent the correct ACK service now
              // the ACK info must be sent latest 1,7 ms after receiving the address type octet of an addressed frame
              _serial.write(TPUART_RX_ACK_SERVICE_NOT_ADDRESSED);
            }
          } 
          break;

      case RX_EIB_TELEGRAM_RECEPTION_ADDRESSED :
          if (readBytesNb == KNX_TELEGRAM_MAX_SIZE) _rx.state = RX_EIB_TELEGRAM_RECEPTION_LENGTH_INVALID;
          else
          {
          telegram.WriteRawByte(incomingByte,readBytesNb);
          readBytesNb++;
          }
          break;

    //  case RX_EIB_TELEGRAM_RECEPTION_LENGTH_INVALID : break; // if the message is too long, nothing to do except waiting for EOP
    //  case RX_EIB_TELEGRAM_RECEPTION_NOT_ADDRESSED : break; // if the message is not addressed, nothing to do except waiting for EOP

      default : break;
    } // switch (_rx.state)
  } // if (_serial.available() > 0)
}


// Transmission task
// This function shall be called periodically in order to allow a correct transmission of the EIB bus data
// Assuming the TP-Uart speed is configured to 19200 baud, a character (8 data + 1 start + 1 parity + 1 stop)
// is transmitted in 0,58ms.
// Sending one byte of a telegram consists in transmitting 2 characters (1,16ms)
// Let's wait around 800us between each telegram piece sending so that the 64byte TX buffer remains almost empty.
// Typical calling period is 800 usec.
void KnxTpUart::TXTask(void)
{
word nowTime;
byte txByte[2];
static word sentMessageTimeMillisec;

  // STEP 1 : Manage Message Acknowledge timeout
  switch (_tx.state)
  {
  case TX_WAITING_ACK :
    // A transmission ACK is awaited, increment Acknowledge timeout
    nowTime = (word) millis(); // word is enough to count up to 500
    if(TimeDeltaWord(nowTime,sentMessageTimeMillisec) > 500 /* 500 ms */ )
    { // The no-answer timeout value is defined as follows :
      // - The emission duration for a single max sized telegram is 40ms
      // - The telegram emission might be repeated 3 times (120ms) 
      // - The telegram emission might be delayed by another message transmission ongoing
      // - The telegram emission might be delayed by the simultaneous transmission of higher prio messages
      // Let's take around 3 times the max emission duration (160ms) as arbitrary value
      _tx.ackFctPtr(NO_ANSWER_TIMEOUT); // Send a No Answer TIMEOUT
      _tx.state = TX_IDLE;
    }
    break;
	
  case TX_TELEGRAM_SENDING_ONGOING :
    // STEP 2 : send message if any to send
    // In case a telegram reception has just started, and the ACK has not been sent yet,
    // we block the transmission (for around 3,3ms) till the ACK is sent
    // In that way, the TX buffer will remain empty and the ACK will be sent immediately
    if (_rx.state != RX_EIB_TELEGRAM_RECEPTION_STARTED)
    {
      {
        if (_tx.nbRemainingBytes == 1)
        { // We are sending the last byte, i.e checksum
          txByte[0] = TPUART_DATA_END_REQ + _tx.txByteIndex;
          txByte[1] = _tx.sentTelegram->ReadRawByte(_tx.txByteIndex);
          _serial.write(txByte,2); // write the UART control field and the data byte

          // Message sending completed
          sentMessageTimeMillisec = (word)millis(); // memorize sending time in order to manage ACK timeout
	  _tx.state = TX_WAITING_ACK;
        }
        else
        {
          txByte[0] = TPUART_DATA_START_CONTINUE_REQ + _tx.txByteIndex;
          txByte[1] = _tx.sentTelegram->ReadRawByte(_tx.txByteIndex);
          _serial.write(txByte,2); // write the UART control field and the data byte
          _tx.txByteIndex++;
          _tx.nbRemainingBytes--;
        }
      }
    }
    break;
	
  default : break;
  } // switch
}


// Get Bus monitoring data (BUS MONITORING mode)
// The function returns true if a new data has been retrieved (data pointer in argument), else false
// It shall be called periodically (max period of 0,5ms) in order to allow correct data reception
// Typical calling period is 400 usec.
boolean KnxTpUart::GetMonitoringData(type_MonitorData& data)
{
word nowTime;
static type_MonitorData currentData={true,0};
static word lastByteRxTimeMicrosec;

  // STEP 1 : Check EOP
  if (!(currentData.isEOP)) // check that we have not already detected an EOP
  {
    nowTime = (word) micros(); // word cast because a 65ms counter is enough
    if(TimeDeltaWord(nowTime,lastByteRxTimeMicrosec) > 2000 /* 2 ms */ )
    {  // EOP detected
      currentData.isEOP = true;
      currentData.dataByte = 0;
      data= currentData;
      return true;
    }
  }
  // STEP 2 : Get New RX Data
  if (_serial.available() > 0) 
  {
    currentData.dataByte = (byte)(_serial.read());
    currentData.isEOP = false;
    data= currentData;
    lastByteRxTimeMicrosec = (word) micros();
    return true;
  }
  return false; // No data received
}


// Check if the target address is an assigned com object one
// if yes, then update index parameter with the index (in the list) of the targeted com object and return true
// else return false
boolean KnxTpUart::IsAddressAssigned(word addr, byte &index) const
{
byte divisionCounter=0;
byte i, searchIndexStart, searchIndexStop, searchIndexRange;

  if (!_assignedComObjectsNb) return false; // in case of empty list, we return immediately

  // Define how many divisions by 2 shall be done in order to reduce the search list by 8 Addr max
  // if _assignedComObjectsNb >= 16 => divisionCounter = 1
  // if _assignedComObjectsNb >= 32 => divisionCounter = 2
  // if _assignedComObjectsNb >= 64 => divisionCounter = 3
  // if _assignedComObjectsNb >= 128 => divisionCounter = 4    
  for (i=4; _assignedComObjectsNb >>i ; i++) divisionCounter++; 

  // the starting point is to search on the whole address range (0 -> _assignedComObjectsNb -1)
  searchIndexStart = 0; searchIndexStop = _assignedComObjectsNb - 1; searchIndexRange = _assignedComObjectsNb;
  
  // reduce the address range if needed
  while(divisionCounter)
  { 
    searchIndexRange>>=1; // Divide range width by 2
    if ( addr >= _comObjectsList[_orderedIndexTable[searchIndexStart+searchIndexRange]].GetAddr())
      searchIndexStart += searchIndexRange ;
    else searchIndexStop-=searchIndexRange;
    divisionCounter --;
  }
  
  // search the address value and index in the reduced range
  for (i = searchIndexStart; ((_comObjectsList[_orderedIndexTable[i]].GetAddr() != addr) && (i <= searchIndexStop)); i++);
  if (i > searchIndexStop) return false; // Address is NOT part of the assigned addresses
  // Address is part of the assigned addresses
  index = _orderedIndexTable[i];
  return true;
}


// DEBUG purpose functions
void KnxTpUart::DEBUG_SendResetCommand() { _serial.write(TPUART_RESET_REQ); }

void KnxTpUart::DEBUG_SendStateReqCommand() { _serial.write(TPUART_STATE_REQ); }

//EOF
