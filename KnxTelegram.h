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


// File : KnxTelegram.h
// Author : Franck Marini
// Description : Handling of the KNX Telegrams
// Module dependencies : none

#ifndef KNXTELEGRAM_H
#define KNXTELEGRAM_H

#include "Arduino.h"

// ---------- Knx Telegram description (visit "www.knx.org" for more info) -----------
// => Length : 9 bytes min. to 23 bytes max.
//
// => Structure :
//      -Header (6 bytes):
//        Byte 0 | Control Field
//        Byte 1 | Source Address High byte
//        Byte 2 | Source Address Low byte
//        Byte 3 | Destination Address High byte
//        Byte 4 | Destination Address Low byte
//        Byte 5 | Routing field
//      -Payload (from 2 up to 16 bytes):
//        Byte 6 | Commmand field High
//        Byte 7 | Command field Low + 1st payload data (6bits)
//        Byte 8 up to 21 | payload bytes (optional)
//      -Checksum (1 byte)
//
// => Fields details :
//      -Control Field : "FFR1 PP00" format with
//         FF = Frame Format (10 = Std Length L_DATA service, 00 = extended L_DATA service, 11 = L_POLLDATA service)
//          R = Repeatflag (1 = not repeated, 0 = repeated)
//         PP = Priority (00 = system, 10 = alarm, 01 = high, 11 = normal)
//      -Routing Field  : "TCCC LLLL" format with
//          T = Target Addr type (1 = group address/muticast, 0 = individual address/unicast)
//        CCC = Counter
//       LLLL = Payload Length (1-15)
//      -Command Field : "00XX XXCC CCDD DDDD" format with
//         XX = Not used
//         CC = command (0000 = Value Read, 0001 = Value Response, 0010 = Value Write, 1010 = Memory Write)
//         DD = Payload Data (1st payload byte)
//
// => Transmit timings :
//     -Tbit = 104us, Tbyte=1,35ms (13 bits per character)
//     -from 20ms for 1 byte payload telegram (Bus temporisation + Telegram transmit + ACK)
//     -up to 40ms for 15 bytes payload (Bus temporisation + Telegram transmit + ACK)
//

// Define for lengths & offsets
#define KNX_TELEGRAM_HEADER_SIZE        6
#define KNX_TELEGRAM_PAYLOAD_MAX_SIZE  16
#define KNX_TELEGRAM_MIN_SIZE           9
#define KNX_TELEGRAM_MAX_SIZE          23
#define KNX_TELEGRAM_LENGTH_OFFSET      8 // Offset between payload length and telegram length

enum e_KnxPriority {
  KNX_PRIORITY_SYSTEM_VALUE  = B00000000,
  KNX_PRIORITY_HIGH_VALUE    = B00000100,
  KNX_PRIORITY_ALARM_VALUE   = B00001000,
  KNX_PRIORITY_NORMAL_VALUE  = B00001100
};

enum e_KnxCommand {
  KNX_COMMAND_VALUE_READ     = B00000000,
  KNX_COMMAND_VALUE_RESPONSE = B00000001,
  KNX_COMMAND_VALUE_WRITE    = B00000010,
  KNX_COMMAND_MEMORY_WRITE   = B00001010
};

//--- CONTROL FIELD values & masks ---
#define CONTROL_FIELD_DEFAULT_VALUE         B10111100 // Standard FF; No Repeat; Normal Priority
#define CONTROL_FIELD_FRAME_FORMAT_MASK     B11000000
#define CONTROL_FIELD_STANDARD_FRAME_FORMAT B10000000
#define CONTROL_FIELD_REPEATED_MASK         B00100000
#define CONTROL_FIELD_SET_REPEATED(x)       (x&=B11011111)
#define CONTROL_FIELD_PRIORITY_MASK         B00001100
#define CONTROL_FIELD_PATTERN_MASK          B00010011
#define CONTROL_FIELD_VALID_PATTERN         B00010000

// --- ROUTING FIELD values & masks ---
#define ROUTING_FIELD_DEFAULT_VALUE            B11100001 // Multicast(Target Group @), Routing Counter = 6, Length = 1
#define ROUTING_FIELD_TARGET_ADDRESS_TYPE_MASK B10000000
#define ROUTING_FIELD_COUNTER_MASK             B01110000 
#define ROUTING_FIELD_PAYLOAD_LENGTH_MASK      B00001111

// --- COMMAND FIELD values & masks ---
#define COMMAND_FIELD_HIGH_COMMAND_MASK 0x03 
#define COMMAND_FIELD_LOW_COMMAND_MASK  0xC0 // 2 first bytes on _commandL
#define COMMAND_FIELD_LOW_DATA_MASK     0x3F // 6 last bytes are data
#define COMMAND_FIELD_PATTERN_MASK      B11000000
#define COMMAND_FIELD_VALID_PATTERN     B00000000

enum e_KnxTelegramValidity { KNX_TELEGRAM_VALID = 0 ,
                             KNX_TELEGRAM_INVALID_CONTROL_FIELD,
                             KNX_TELEGRAM_UNSUPPORTED_FRAME_FORMAT,
                             KNX_TELEGRAM_INCORRECT_PAYLOAD_LENGTH,
                             KNX_TELEGRAM_INVALID_COMMAND_FIELD,
                             KNX_TELEGRAM_UNKNOWN_COMMAND,
                             KNX_TELEGRAM_INCORRECT_CHECKSUM };

class KnxTelegram {
    union {
        byte _telegram[KNX_TELEGRAM_MAX_SIZE]; // byte 0 to 22
        struct {
        byte _controlField; // byte 0
        byte _sourceAddrH;  // byte 1
        byte _sourceAddrL;  // byte 2
        byte _targetAddrH;  // byte 3
        byte _targetAddrL;  // byte 4
        byte _routing;      // byte 5
        byte _commandH;     // byte 6
        byte _commandL;     // byte 7
        byte _payloadChecksum[KNX_TELEGRAM_PAYLOAD_MAX_SIZE-1]; // byte 8 to 22
      };
    };

  public:
  // CONSTRUCTOR
    // builds telegram with following default values :
    // std FF, no repeat, normal prio, empty payload, multicast, routing counter = 6, payload length = 1
    KnxTelegram();
    
  // INLINED functions (defined later in this file)
    void ChangePriority(e_KnxPriority priority);
    e_KnxPriority GetPriority(void) const;

    void SetRepeated(void);
    boolean IsRepeated(void) const;

    void SetSourceAddress(word addr);
    word GetSourceAddress(void) const;
    void SetTargetAddress(word addr);
    word GetTargetAddress(void) const;

    void SetMulticast(boolean);
    boolean IsMulticast(void) const;

    void ChangeRoutingCounter(byte counter);
    byte GetRoutingCounter(void) const;

    void SetPayloadLength(byte length);
    byte GetPayloadLength(void) const;

    byte GetTelegramLength(void) const;

    void SetCommand(e_KnxCommand cmd);
    e_KnxCommand GetCommand(void) const;

    // Handling of the 1st payload byte (the 6 lowest bits in _commandL field)
    void SetFirstPayloadByte(byte data);
    void ClearFirstPayloadByte(void);
    byte GetFirstPayloadByte(void) const;

    // Read of the telegram byte per byte
    // NB : do not check that the index is in the range
    byte ReadRawByte(byte byteIndex) const;

    // Write of the telegram byte per byte
    // NB : do not check that the index is in the range
    void WriteRawByte(byte data, byte byteIndex);

    byte GetChecksum(void) const;
    boolean IsChecksumCorrect(void) const;

  // functions NOT INLINED (see definitions in KnxTelegram.cpp)
    void ClearTelegram(void); // (re)set telegram with default values

    // Set 'nbOfBytes' bytes of the payload starting from the 2nd payload byte
    // if 'nbOfBytes' val is out of range, then we use the max allowed value instead
    void SetLongPayload(const byte origin[], byte  nbOfBytes);
    // Get 'nbOfBytes' bytes of the payload starting from the 2nd payload byte
    // if 'nbOfBytes' val  is out of range, then we use the max allowed value instead
    void GetLongPayload(byte destination[], byte nbOfBytes) const;

    // Clear the whole payload except the 1st payload byte
    void ClearLongPayload(void);

    byte CalculateChecksum(void) const;
    // Let the class calculate and update the proper checksum value in the telegram
    void UpdateChecksum(void);

    // Whole telegram copy
    void Copy(KnxTelegram& dest) const;
    // Header Copy (6 1st bytes of the telegram)
    void CopyHeader(KnxTelegram& dest) const;

    e_KnxTelegramValidity GetValidity(void) const;

  // DEBUG functions :
    void Info(String&) const; // copy telegram info into a string
    void InfoRaw(String&) const; // copy raw data telegram into a string
    void InfoVerbose(String&) const; // copy verbose telegram info into a string
};


// --------------- Definition of the INLINED functions : -----------------
inline void KnxTelegram::ChangePriority(e_KnxPriority priority)
{ _controlField &= ~CONTROL_FIELD_PRIORITY_MASK; _controlField |= priority & CONTROL_FIELD_PRIORITY_MASK;}
    
inline e_KnxPriority KnxTelegram::GetPriority(void) const 
{return (e_KnxPriority)(_controlField & CONTROL_FIELD_PRIORITY_MASK);}

inline void KnxTelegram::SetRepeated(void ) 
{ CONTROL_FIELD_SET_REPEATED(_controlField);};
    
inline boolean KnxTelegram::IsRepeated(void) const 
{if (_controlField & CONTROL_FIELD_REPEATED_MASK ) return false; else return true ; }

inline void KnxTelegram::SetSourceAddress(word addr) { 
  // WARNING : works with little endianness only
  // The adresses within KNX telegram are big endian
  _sourceAddrL = (byte) addr; _sourceAddrH = byte(addr>>8);}

inline word KnxTelegram::GetSourceAddress(void) const {
  // WARNING : works with little endianness only
  // The adresses within KNX telegram are big endian
  word addr; addr = _sourceAddrL + (_sourceAddrH<<8); return addr; }

inline void KnxTelegram:: SetTargetAddress(word addr) { 
  // WARNING : works with little endianness only
  // The adresses within KNX telegram are big endian
  _targetAddrL = (byte) addr; _targetAddrH = byte(addr>>8);}

inline word KnxTelegram::GetTargetAddress(void) const {
 // WARNING : endianess sensitive!! Code below is for LITTLE ENDIAN chip
 // The KNX telegram uses BIG ENDIANNESS (Hight byte placed before Low Byte)
  word addr; addr = _targetAddrL + (_targetAddrH<<8); return addr; }

inline boolean KnxTelegram::IsMulticast(void) const 
{return (_routing & ROUTING_FIELD_TARGET_ADDRESS_TYPE_MASK);}

inline void KnxTelegram::SetMulticast(boolean mode)
{ if (mode) _routing|= ROUTING_FIELD_TARGET_ADDRESS_TYPE_MASK;
  else _routing &= ~ROUTING_FIELD_TARGET_ADDRESS_TYPE_MASK; }
 
inline void KnxTelegram::ChangeRoutingCounter(byte counter) 
{ counter <<= 4; _routing &= ~ROUTING_FIELD_COUNTER_MASK; _routing |= (counter & ROUTING_FIELD_COUNTER_MASK); }

inline byte KnxTelegram::GetRoutingCounter(void) const 
{ return ((_routing & ROUTING_FIELD_COUNTER_MASK)>>4); }

inline void KnxTelegram::SetPayloadLength(byte length) 
{ _routing&= ~ROUTING_FIELD_PAYLOAD_LENGTH_MASK ; _routing |= length & ROUTING_FIELD_PAYLOAD_LENGTH_MASK; }

inline byte KnxTelegram::GetPayloadLength(void) const 
{return (_routing & ROUTING_FIELD_PAYLOAD_LENGTH_MASK);}

inline byte KnxTelegram::GetTelegramLength(void) const 
{ return (KNX_TELEGRAM_LENGTH_OFFSET + GetPayloadLength());}

inline void KnxTelegram::SetCommand(e_KnxCommand cmd) {
  _commandH &= ~COMMAND_FIELD_HIGH_COMMAND_MASK; _commandH |= (cmd >> 2);
  _commandL &= ~COMMAND_FIELD_LOW_COMMAND_MASK;  _commandL |= (cmd << 6);}

inline e_KnxCommand KnxTelegram::GetCommand(void) const 
{return (e_KnxCommand)(((_commandL & COMMAND_FIELD_LOW_COMMAND_MASK)>>6) + ((_commandH & COMMAND_FIELD_HIGH_COMMAND_MASK)<<2)); };
    
inline void KnxTelegram::SetFirstPayloadByte(byte data) 
{ _commandL &= ~COMMAND_FIELD_LOW_DATA_MASK ; _commandL |= data & COMMAND_FIELD_LOW_DATA_MASK; }

inline void KnxTelegram::ClearFirstPayloadByte(void)
{ _commandL &= ~COMMAND_FIELD_LOW_DATA_MASK;}

inline byte KnxTelegram::GetFirstPayloadByte(void) const 
{ return (_commandL & COMMAND_FIELD_LOW_DATA_MASK);}

inline byte KnxTelegram::ReadRawByte(byte byteIndex) const
{ return _telegram[byteIndex];}

inline void KnxTelegram::WriteRawByte(byte data, byte byteIndex)
{ _telegram[byteIndex] = data;}

inline byte KnxTelegram::GetChecksum(void) const 
{ return (_payloadChecksum[GetPayloadLength() - 1]);}

inline boolean KnxTelegram::IsChecksumCorrect(void) const 
{ return (GetChecksum()==CalculateChecksum());}

#endif // KNXTELEGRAM_H
