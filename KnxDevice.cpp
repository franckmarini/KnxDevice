//    This file is part of Arduino Knx Bus Device library.

//    The Arduino Knx Bus Device library allows to turn Arduino into "self-made" KNX bus device.
//    Copyright (C) 2014  Franck MARINI (fm@liwan.fr)

//    The Arduino Knx Bus Device library is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Publitkic License for more details.

//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

// File : KnxDevice.cpp
// Author : Franck Marini
// Description : KnxDevice Abstraction Layer
// Module dependencies : HardwareSerial, KnxTelegram, KnxComObject, KnxTpUart, RingBuffer

#include "KnxDevice.h"

static inline word TimeDeltaWord(word now, word before) { return (word)(now - before); }

#ifdef KNXDEVICE_DEBUG_INFO
const char KnxDevice::_debugInfoText[] = "KNXDEVICE INFO: ";
#endif

// KnxDevice unique instance creation
KnxDevice KnxDevice::Knx;
KnxDevice& Knx = KnxDevice::Knx;


// Constructor
KnxDevice::KnxDevice()
{
  _state = INIT;
  _tpuart = NULL;
  _txActionList= RingBuffer<type_tx_action, ACTIONS_QUEUE_SIZE>();
  _initCompleted = false;
  _initIndex = 0;
  _rxTelegram = NULL;
#if defined(KNXDEVICE_DEBUG_INFO)
   _nbOfInits = 0;
   _debugStrPtr = NULL;
#endif
}


// Start the KNX Device
// return KNX_DEVICE_ERROR (255) if begin() failed
// else return KNX_DEVICE_OK
byte KnxDevice::begin(HardwareSerial& serial, word physicalAddr)
{
  _tpuart = new KnxTpUart(serial ,physicalAddr, NORMAL);
  _rxTelegram = &_tpuart->GetReceivedTelegram();
  if(_tpuart->Reset()!= KNX_TPUART_OK)
  {
    delete(_tpuart);
    _tpuart = NULL;
    _rxTelegram = NULL;
    DebugInfo("Init Error!\n");
    return KNX_DEVICE_ERROR;
  }
  _tpuart->AttachComObjectsList(_comObjectsList, _comObjectsNb);
  _tpuart->SetEvtCallback(&KnxDevice::GetTpUartEvents);
  _tpuart->SetAckCallback(&KnxDevice::TxTelegramAck);
  _tpuart->Init();
  _state = IDLE;
  DebugInfo("Init successful\n");
  _lastInitTimeMillis = millis();
  _lastTXTimeMicros = _lastTXTimeMicros = micros();
#if defined(KNXDEVICE_DEBUG_INFO)
   _nbOfInits = 0;
#endif
  return KNX_DEVICE_OK;
}


// Stop the KNX Device
void KnxDevice::end()
{
type_tx_action action;

  _state = INIT;
  while(_txActionList.Pop(action)); // empty ring buffer
  _initCompleted = false;
  _initIndex = 0;
  _rxTelegram = NULL;
  delete(_tpuart);
  _tpuart = NULL;
}


// KNX device execution task
// This function call shall be placed in the "loop()" Arduino function
void KnxDevice::task(void)
{
type_tx_action action;
word nowTimeMillis, nowTimeMicros;

  // STEP 1 : Initialize Com Objects having Init Read attribute
  if(!_initCompleted)
  { 
    nowTimeMillis = millis();
    // To avoid EIB bus overloading, we wait for 500 ms between each Init read request
    if (TimeDeltaWord(nowTimeMillis, _lastInitTimeMillis) > 500 )
    { 
      while ( (_initIndex< _comObjectsNb) && (_comObjectsList[_initIndex].GetValidity() )) _initIndex++;

      if (_initIndex == _comObjectsNb) 
      {
        _initCompleted = true; // All the Com Object initialization have been performed
      //  DebugInfo(String("KNXDevice INFO: Com Object init completed, ")+ String( _nbOfInits) + String("objs initialized.\n"));
      }
      else 
      { // Com Object to be initialised has been found
        // Add a READ request in the TX action list
#if defined(KNXDEVICE_DEBUG_INFO) || defined(KNXDEVICE_DEBUG_INFO_VERBOSE)
        _nbOfInits++;
#endif
        action.command = EIB_READ_REQUEST;
        action.index = _initIndex;
        _txActionList.Append(action);
        _lastInitTimeMillis = millis(); // Update the timer
      }
    } 
  }

  // STEP 2 : Get new received EIB messages from the TPUART
  // The TPUART RX task is executed every 400 us
  nowTimeMicros = micros();
  if (TimeDeltaWord(nowTimeMicros, _lastRXTimeMicros) > 400)
  {
    _lastRXTimeMicros = nowTimeMicros;
    _tpuart->RXTask();
  }

  // STEP 3 : Send KNX messages following TX actions
  if(_state == IDLE)
  {
    if( _txActionList.Pop(action))
    { // Data to be transmitted
      switch (action.command)
      {
        case EIB_READ_REQUEST: // a read operation of a Com Object on the EIB network is required
          //_objectsList[action.index].CopyToTelegram(_txTelegram, KNX_COMMAND_VALUE_READ);
          _comObjectsList[action.index].CopyAttributes(_txTelegram);
          _txTelegram.ClearLongPayload(); _txTelegram.ClearFirstPayloadByte(); // Is it required to have a clean payload ??
          _txTelegram.SetCommand(KNX_COMMAND_VALUE_READ);
          _txTelegram.UpdateChecksum();
          _tpuart->SendTelegram(_txTelegram);
          _state = TX_ONGOING;
          break;

        case EIB_RESPONSE_REQUEST: // a response operation of a Com Object on the EIB network is required
          _comObjectsList[action.index].CopyAttributes(_txTelegram);
          _comObjectsList[action.index].CopyValue(_txTelegram);
          _txTelegram.SetCommand(KNX_COMMAND_VALUE_RESPONSE);
          _txTelegram.UpdateChecksum();
          _tpuart->SendTelegram(_txTelegram);
          _state = TX_ONGOING;
          break;

        case EIB_WRITE_REQUEST: // a write operation of a Com Object on the EIB network is required
          // update the com obj value
          if ((_comObjectsList[action.index].GetLength()) <= 2 )
            _comObjectsList[action.index].UpdateValue(action.byteValue);
          else
          {
            _comObjectsList[action.index].UpdateValue(action.valuePtr);
            delete(action.valuePtr);
          }
          // transmit the value through EIB network only if the Com Object has transmit attribute
          if ( (_comObjectsList[action.index].GetIndicator()) & KNX_COM_OBJ_T_INDICATOR)
          {
            _comObjectsList[action.index].CopyAttributes(_txTelegram);
            _comObjectsList[action.index].CopyValue(_txTelegram);
            _txTelegram.SetCommand(KNX_COMMAND_VALUE_WRITE);
            _txTelegram.UpdateChecksum();
            _tpuart->SendTelegram(_txTelegram);
            _state = TX_ONGOING;
          }
          break;

        default : break;
      }
    }
  }
  
  // STEP 4 : LET THE TP-UART TRANSMIT EIB MESSAGES
  // The TPUART TX task is executed every 800 us
  nowTimeMicros = micros();
  if (TimeDeltaWord(nowTimeMicros, _lastTXTimeMicros) > 800)
  {
    _lastTXTimeMicros = nowTimeMicros;
    _tpuart->TXTask();
  }
}


// Read short value (<=1 byte) com object
byte KnxDevice::read(byte objectIndex)
{
  return _comObjectsList[objectIndex].GetValue();
}


// Read long value (> 1 byte) com object
void KnxDevice::read(byte objectIndex, byte returnedValue[])
{
  _comObjectsList[objectIndex].GetValue(returnedValue);
}


// Write short value (<=1 byte) value com object
// The Com Object value is updated locally
// And a telegram is sent on the EIB bus if the com object has communication & transmit attributes
void KnxDevice::write(byte objectIndex, const byte byteValue)
{
type_tx_action action;
  //step 1 : check that Com Object payload length is less or equals 1 byte
  if (_comObjectsList[objectIndex].GetLength()<=2)
  { // add WRITE action in the TX action queue
    action.command = EIB_WRITE_REQUEST;
    action.index = objectIndex;
    action.byteValue = byteValue;
    _txActionList.Append(action);
  }
}


// Write long value (> 1 byte) com object
// The Com Object value is updated locally
// And a telegram is sent on the EIB bus if the com object has communication & transmit attributes
void KnxDevice::write(byte objectIndex, const byte valuePtr[])
{
type_tx_action action;
  //step 1 : check that Com Object payload length is more than 2 bytes
  if (_comObjectsList[objectIndex].GetLength()>2)
  { // add WRITE action in the TX action queue
    action.command = EIB_WRITE_REQUEST;
    action.index = objectIndex;
    action.valuePtr = (byte *) valuePtr;
    _txActionList.Append(action);
  }
}


// Com Object Update request
// Request the local object to be updated with the value from the bus
// NB : the function is asynchroneous, the update completion is notified by the knxEvents() callback
void KnxDevice::update(byte objectIndex)
{
type_tx_action action;
  action.command = EIB_READ_REQUEST;
  action.index = objectIndex;
  _txActionList.Append(action); 
}


// The function returns true if there is rx/tx activity ongoing, else false
boolean KnxDevice::isActive(void) const
{
  if (_tpuart->IsActive()) return true; // TPUART is active
  if (_state == TX_ONGOING) return true; // the Device is sending a request
  if(_txActionList.ElementsNb()) return true; // there is at least one tx action in the queue
  return false;
}


// Static GetTpUartEvents() function called by the KnxTpUart layer (callback)
void KnxDevice::GetTpUartEvents(e_KnxTpUartEvent event)
{
type_tx_action action;
byte targetedComObjIndex; // index of the Com Object targeted by the event

  // Manage RECEIVED MESSAGES
  if (event == TPUART_EVENT_RECEIVED_EIB_TELEGRAM)
  {
    Knx._state = IDLE;
    targetedComObjIndex = Knx._tpuart->GetTargetedComObjectIndex();

    switch(Knx._rxTelegram->GetCommand())
    {
      case KNX_COMMAND_VALUE_READ :
    	Knx.DebugInfo("READ req.\n");
        // READ command coming from the bus
        // if the Com Object has read attribute, then add RESPONSE action in the TX action list
        if ( (_comObjectsList[targetedComObjIndex].GetIndicator()) & KNX_COM_OBJ_R_INDICATOR)
        { // The targeted Com Object can indeed be read
          action.command = EIB_RESPONSE_REQUEST;
          action.index = targetedComObjIndex;
          Knx._txActionList.Append(action);
        }
        break;

      case KNX_COMMAND_VALUE_RESPONSE :
      	Knx.DebugInfo("RESP req.\n");
        // RESPONSE command coming from EIB network, we update the value of the corresponding Com Object.
        // We 1st check that the corresponding Com Object has UPDATE attribute
        if((_comObjectsList[targetedComObjIndex].GetIndicator()) & KNX_COM_OBJ_U_INDICATOR)
        {
          _comObjectsList[targetedComObjIndex].UpdateValue(*(Knx._rxTelegram));
          //We notify the upper layer of the update
          knxEvents(targetedComObjIndex);
        }
        break;


      case KNX_COMMAND_VALUE_WRITE :
    	Knx.DebugInfo("WRITE req.\n");
        // WRITE command coming from EIB network, we update the value of the corresponding Com Object.
        // We 1st check that the corresponding Com Object has WRITE attribute
        if((_comObjectsList[targetedComObjIndex].GetIndicator()) & KNX_COM_OBJ_W_INDICATOR)
        {
          _comObjectsList[targetedComObjIndex].UpdateValue(*(Knx._rxTelegram));
          //We notify the upper layer of the update
          knxEvents(targetedComObjIndex);
        }
        break;

      // case KNX_COMMAND_MEMORY_WRITE : break; // Memory Write not handled

      default : break; // not supposed to happen
    }
  }

  // Manage RESET events
  if (event == TPUART_EVENT_RESET)
  {
    while(Knx._tpuart->Reset()==KNX_TPUART_ERROR);
    Knx._tpuart->Init();
    Knx._state = IDLE;
  }
}


// Static TxTelegramAck() function called by the KnxTpUart layer (callback)
void KnxDevice::TxTelegramAck(e_TpUartTxAck value)
{
  Knx._state = IDLE;
#ifdef KNXDevice_DEBUG
  if(value != ACK_RESPONSE)
  {
    switch(value)
    {  
      case NACK_RESPONSE: DebugInfo("NACK RESPONSE!!\n"); break;
      case NO_ANSWER_TIMEOUT: DebugInfo("NO ANSWER TIMEOUT RESPONSE!!\n");; break;
      case TPUART_RESET_RESPONSE: DebugInfo("RESET RESPONSE!!\n");; break;
    }
  }
#endif // KNXDevice_DEBUG
} 

// EOF

