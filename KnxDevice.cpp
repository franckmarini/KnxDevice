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

// File : KnxDevice.cpp
// Author : Franck Marini
// Description : KnxDevice Abstraction Layer
// Module dependencies : HardwareSerial, KnxTelegram, KnxComObject, KnxTpUart, ActionRingBuffer

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
  _txActionList= ActionRingBuffer<type_tx_action, ACTIONS_QUEUE_SIZE>();
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
e_KnxDeviceStatus KnxDevice::begin(HardwareSerial& serial, word physicalAddr)
{
  _tpuart = new KnxTpUart(serial ,physicalAddr, NORMAL);
  _rxTelegram = &_tpuart->GetReceivedTelegram();
  // delay(10000); // Workaround for init issue with bus-powered arduino
                   // the issue is reproduced on one (faulty?) TPUART device only, so remove it for the moment.
  if(_tpuart->Reset()!= KNX_TPUART_OK)
  {
    delete(_tpuart);
    _tpuart = NULL;
    _rxTelegram = NULL;
#if defined(KNXDEVICE_DEBUG_INFO)
    DebugInfo("Init Error!\n");
#endif
    return KNX_DEVICE_ERROR;
  }
  _tpuart->AttachComObjectsList(_comObjectsList, _comObjectsNb);
  _tpuart->SetEvtCallback(&KnxDevice::GetTpUartEvents);
  _tpuart->SetAckCallback(&KnxDevice::TxTelegramAck);
  _tpuart->Init();
  _state = IDLE;
#if defined(KNXDEVICE_DEBUG_INFO)
  DebugInfo("Init successful\n");
#endif
  _lastInitTimeMillis = millis();
  _lastTXTimeMicros = micros();
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
            free(action.valuePtr);
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


// Quick method to read a short (<=1 byte) com object
// NB : The returned value will be hazardous in case of use with long objects
byte KnxDevice::read(byte objectIndex)
{
  return _comObjectsList[objectIndex].GetValue();
}


// Read an usual format com object
// Supported DPT formats are short com object, U16, V16, U32, V32, F16 and F32 (not implemented yet)
template <typename T>  e_KnxDeviceStatus KnxDevice::read(byte objectIndex, T& returnedValue)
{
  // Short com object case
  if (_comObjectsList[objectIndex].GetLength()<=2)
  {
    returnedValue = (T) _comObjectsList[objectIndex].GetValue();
    return KNX_DEVICE_OK;
  }
  else // long object case, let's see if we are able to translate the DPT value
  {
    byte dptValue[14]; // define temporary DPT value with max length
    _comObjectsList[objectIndex].GetValue(dptValue);
    return ConvertFromDpt(dptValue, returnedValue, pgm_read_byte(&KnxDPTIdToFormat[_comObjectsList[objectIndex].GetDptId()]));
  }
}

template e_KnxDeviceStatus KnxDevice::read <boolean>(byte objectIndex, boolean& returnedValue);
template e_KnxDeviceStatus KnxDevice::read <unsigned char>(byte objectIndex, unsigned char& returnedValue);
template e_KnxDeviceStatus KnxDevice::read <char>(byte objectIndex, char& returnedValue);
template e_KnxDeviceStatus KnxDevice::read <unsigned int>(byte objectIndex, unsigned int& returnedValue);
template e_KnxDeviceStatus KnxDevice::read <int>(byte objectIndex, int& returnedValue);
template e_KnxDeviceStatus KnxDevice::read <unsigned long>(byte objectIndex, unsigned long& returnedValue);
template e_KnxDeviceStatus KnxDevice::read <long>(byte objectIndex, long& returnedValue);
template e_KnxDeviceStatus KnxDevice::read <float>(byte objectIndex, float& returnedValue);
template e_KnxDeviceStatus KnxDevice::read <double>(byte objectIndex, double& returnedValue);



// Read any type of com object (DPT value provided as is)
e_KnxDeviceStatus KnxDevice::read(byte objectIndex, byte returnedValue[])
{
  _comObjectsList[objectIndex].GetValue(returnedValue);
  return KNX_DEVICE_OK;
}


// Update an usual format com object
// Supported DPT types are short com object, U16, V16, U32, V32, F16 and F32
// The Com Object value is updated locally
// And a telegram is sent on the EIB bus if the com object has communication & transmit attributes
template <typename T>  e_KnxDeviceStatus KnxDevice::write(byte objectIndex, T value)
{
  type_tx_action action;
  byte *destValue;
  byte length = _comObjectsList[objectIndex].GetLength();
  
  if (length <= 2 ) action.byteValue = (byte) value; // short object case
  else
  { // long object case, let's try to translate value to the com object DPT
    destValue = (byte *) malloc(length-1); // allocate the memory for DPT
    e_KnxDeviceStatus status = ConvertToDpt(value, destValue, pgm_read_byte(&KnxDPTIdToFormat[_comObjectsList[objectIndex].GetDptId()]));
    if (status) // translation error
    { 
      free(destValue);
      return status; // we cannot convert, we stop here
    }
    else  action.valuePtr = destValue;
  }    
  // add WRITE action in the TX action queue
  action.command = EIB_WRITE_REQUEST;
  action.index = objectIndex;
  _txActionList.Append(action);
  return KNX_DEVICE_OK;
}

template e_KnxDeviceStatus KnxDevice::write <boolean>(byte objectIndex, boolean value);
template e_KnxDeviceStatus KnxDevice::write <unsigned char>(byte objectIndex, unsigned char value);
template e_KnxDeviceStatus KnxDevice::write <char>(byte objectIndex, char value);
template e_KnxDeviceStatus KnxDevice::write <unsigned int>(byte objectIndex, unsigned int value);
template e_KnxDeviceStatus KnxDevice::write <int>(byte objectIndex, int value);
template e_KnxDeviceStatus KnxDevice::write <unsigned long>(byte objectIndex, unsigned long value);
template e_KnxDeviceStatus KnxDevice::write <long>(byte objectIndex, long value);
template e_KnxDeviceStatus KnxDevice::write <float>(byte objectIndex, float value);
template e_KnxDeviceStatus KnxDevice::write <double>(byte objectIndex, double value);


// Update any type of com object (rough DPT value shall be provided)
// The Com Object value is updated locally
// And a telegram is sent on the EIB bus if the com object has communication & transmit attributes
e_KnxDeviceStatus KnxDevice::write(byte objectIndex, byte valuePtr[])
{
type_tx_action action;
byte *dptValue;
byte length = _comObjectsList[objectIndex].GetLength();

  if (length>2) // check we are in long object case
  { // add WRITE action in the TX action queue
    action.command = EIB_WRITE_REQUEST;
    action.index = objectIndex;
    dptValue = (byte *) malloc(length-1); // allocate the memory for long value
    for (byte i=0; i<length-1; i++) dptValue[i] = valuePtr[i]; // copy value
    action.valuePtr = (byte *) dptValue;
    _txActionList.Append(action);
    return KNX_DEVICE_OK;
  }
  return KNX_DEVICE_ERROR;
}


// Com Object EIB Bus Update request
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
#if defined(KNXDEVICE_DEBUG_INFO)
    	Knx.DebugInfo("READ req.\n");
#endif
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
#if defined(KNXDEVICE_DEBUG_INFO)
      	Knx.DebugInfo("RESP req.\n");
#endif
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
#if defined(KNXDEVICE_DEBUG_INFO)
    	Knx.DebugInfo("WRITE req.\n");
#endif
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


// Functions to convert a standard C type to a DPT format
// NB : only the usual DPT formats are supported (U16, V16, U32, V32, F16 and F32 (not yet implemented)
template <typename T> e_KnxDeviceStatus ConvertFromDpt(const byte dptOriginValue[], T& resultValue, byte dptFormat)
{
  switch (dptFormat)
  {
    case KNX_DPT_FORMAT_U16:
    case KNX_DPT_FORMAT_V16:
      resultValue = (T)((unsigned int)dptOriginValue[0] << 8);
      resultValue += (T)(dptOriginValue[1]);
      return KNX_DEVICE_OK;
    break;

    case KNX_DPT_FORMAT_U32:
    case KNX_DPT_FORMAT_V32:
      resultValue = (T)((unsigned long)dptOriginValue[0] << 24);
      resultValue += (T)((unsigned long)dptOriginValue[1] << 16);
      resultValue += (T)((unsigned long)dptOriginValue[2] << 8);
      resultValue += (T)(dptOriginValue[3]);
      return KNX_DEVICE_OK;
    break;

    case KNX_DPT_FORMAT_F16 :
    {
      // Get the DPT sign, mantissa and exponent
      int signMultiplier = (dptOriginValue[0] & 0x80) ? -1 : 1;
      word absoluteMantissa = dptOriginValue[1] + ((dptOriginValue[0]&0x07)<<8);
      if (signMultiplier == -1) 
      { // Calculate absolute mantissa value in case of negative mantissa
        // Abs = 2's complement + 1
        absoluteMantissa = ((~absoluteMantissa)& 0x07FF ) + 1;
      }
      byte exponent = (dptOriginValue[0]&0x78)>>3;
      resultValue = (T)(0.01 * ((long)absoluteMantissa << exponent) * signMultiplier);
      return KNX_DEVICE_OK;
    }
    break;

    case KNX_DPT_FORMAT_F32 :
      return KNX_DEVICE_NOT_IMPLEMENTED;
    break;

    default :
      return KNX_DEVICE_ERROR;
    break;
  }
}

template e_KnxDeviceStatus ConvertFromDpt <unsigned char>(const byte dptOriginValue[], unsigned char&, byte dptFormat);
template e_KnxDeviceStatus ConvertFromDpt <char>(const byte dptOriginValue[], char&, byte dptFormat);
template e_KnxDeviceStatus ConvertFromDpt <unsigned int>(const byte dptOriginValue[], unsigned int&, byte dptFormat);
template e_KnxDeviceStatus ConvertFromDpt <int>(const byte dptOriginValue[], int&, byte dptFormat);
template e_KnxDeviceStatus ConvertFromDpt <unsigned long>(const byte dptOriginValue[], unsigned long&, byte dptFormat);
template e_KnxDeviceStatus ConvertFromDpt <long>(const byte dptOriginValue[], long&, byte dptFormat);
template e_KnxDeviceStatus ConvertFromDpt <float>(const byte dptOriginValue[], float&, byte dptFormat);
template e_KnxDeviceStatus ConvertFromDpt <double>(const byte dptOriginValue[], double&, byte dptFormat);


// Functions to convert a standard C type to a DPT format
// NB : only the usual DPT formats are supported (U16, V16, U32, V32, F16 and F32 (not yet implemented)
template <typename T> e_KnxDeviceStatus ConvertToDpt(T originValue, byte dptDestValue[], byte dptFormat)
{
  switch (dptFormat)
  {
    case KNX_DPT_FORMAT_U16:
    case KNX_DPT_FORMAT_V16:
      dptDestValue[0] = (byte)((unsigned int)originValue>>8);
      dptDestValue[1] = (byte)(originValue);
      return KNX_DEVICE_OK;
    break;

    case KNX_DPT_FORMAT_U32:
    case KNX_DPT_FORMAT_V32:
      dptDestValue[0] = (byte)((unsigned long)originValue>>24);
      dptDestValue[1] = (byte)((unsigned long)originValue>>16);
      dptDestValue[2] = (byte)((unsigned long)originValue>>8);
      dptDestValue[3] = (byte)(originValue);
      return KNX_DEVICE_OK;
    break;

    case KNX_DPT_FORMAT_F16 :
    {
      long longValuex100 = (long)(100.0 * originValue);
      boolean negativeSign = (longValuex100 & 0x80000000)? true : false;
      byte exponent = 0;
      byte round = 0;

      if (negativeSign)
      {
        while(longValuex100 < (long)(-2048)) 
        {
           exponent++; round = (byte)(longValuex100) & 1 ; longValuex100>>=1; longValuex100|=0x80000000;
        }
      }
      else
      {
        while(longValuex100 > (long)(2047)) 
        {
          exponent++; round = (byte)(longValuex100) & 1 ; longValuex100>>=1;
        }

      }
      if (round) longValuex100++;
      dptDestValue[1] = (byte)longValuex100;
      dptDestValue[0] = (byte)(longValuex100>>8) & 0x7 ;
      dptDestValue[0] += exponent<<3;
      if (negativeSign) dptDestValue[0] += 0x80;
      return KNX_DEVICE_OK;
    }
    break;

    case KNX_DPT_FORMAT_F32 :
      return KNX_DEVICE_NOT_IMPLEMENTED;
    break;

    default :
      return KNX_DEVICE_ERROR;
    break;
  }
}

template e_KnxDeviceStatus ConvertToDpt <unsigned char>(unsigned char, byte dptDestValue[], byte dptFormat);
template e_KnxDeviceStatus ConvertToDpt <char>(char, byte dptDestValue[], byte dptFormat);
template e_KnxDeviceStatus ConvertToDpt <unsigned int>(unsigned int, byte dptDestValue[], byte dptFormat);
template e_KnxDeviceStatus ConvertToDpt <int>(int, byte dptDestValue[], byte dptFormat);
template e_KnxDeviceStatus ConvertToDpt <unsigned long>(unsigned long, byte dptDestValue[], byte dptFormat);
template e_KnxDeviceStatus ConvertToDpt <long>(long, byte dptDestValue[], byte dptFormat);
template e_KnxDeviceStatus ConvertToDpt <float>(float, byte dptDestValue[], byte dptFormat);
template e_KnxDeviceStatus ConvertToDpt <double>(double, byte dptDestValue[], byte dptFormat);

// EOF

