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


// File : KnxComObject.cpp
// Author : Franck Marini
// Description : Handling of the KNX Communication Objects
// Module dependencies : KnxTelegram

#include "KnxComObject.h"

// Data length is calculated in the same way as telegram payload length
byte lengthCalculation(e_KnxDPT_ID dptId) {
	return (pgm_read_byte(&KnxDPTFormatToLengthBit[ pgm_read_byte(&KnxDPTIdToFormat[dptId])] ) / 8) + 1;
}


// Contructor
#ifdef KNX_COM_OBJ_SUPPORT_ALL_PRIORITIES
KnxComObject::KnxComObject(word addr, e_KnxDPT_ID dptId, e_KnxPriority prio, byte indicator )
: _addr(addr), _dptId(dptId), _indicator(indicator), _length(lengthCalculation(dptId)), _prio(prio)
#else
KnxComObject::KnxComObject(word addr, e_KnxDPT_ID dptId, byte indicator )
: _addr(addr), _dptId(dptId), _indicator(indicator), _length(lengthCalculation(dptId))
#endif
{
	if (_length <= 2) _longValue = NULL; // short value case
	else { // long value case
		_longValue = (byte *) malloc(_length-1);
		for (byte i=0; i <_length-1 ; i++) _longValue[i] = 0;
	}  
	if (_indicator & KNX_COM_OBJ_I_INDICATOR) _validity = false; // case of object with "InitRead" indicator
	else _validity = true; // case of object without "InitRead" indicator
}


// Destructor
KnxComObject::~KnxComObject() { if (_length > 2) free(_longValue); }



// Get the com obj value (short and long value cases)
void KnxComObject::GetValue(byte dest[]) const
{
	if (_length <=2) dest[0] = _value; // short value case, ReadValue(void) fct should rather be used
	else for (byte i=0; i < _length-1 ; i++) dest[i] = _longValue[i]; // long value case
}


// Update the com obj value (short and long value cases)
void KnxComObject::UpdateValue(const byte ori[])
{
	if (_length <=2) _value = ori[0]; // short value case, UpdateValue(byte) fct should rather be used
	else for (byte i=0; i < _length-1 ; i++) _longValue[i] = ori[i]; // long value case
	_validity = true;  // com obj set to valid
}


// Update the com obj value with the telegram payload content
byte KnxComObject::UpdateValue(const KnxTelegram& ori)
{
	if (ori.GetPayloadLength() != GetLength()) return KNX_COM_OBJECT_ERROR; // Error : telegram payload length differs from com obj one
	if (_length == 1) _value = ori.GetFirstPayloadByte();
	else if (_length == 2) ori.GetLongPayload(&_value,1);
	else ori.GetLongPayload(_longValue, _length - 1);
	_validity = true;  // com object set to valid
	return KNX_COM_OBJECT_OK;
}


// Copy the com obj attributes (addr, prio & length) into a telegram object
void KnxComObject::CopyAttributes(KnxTelegram& dest) const
{
	dest.ChangePriority(GetPriority());
	dest.SetTargetAddress(GetAddr());
	dest.SetPayloadLength(_length); // case short length
}


// Copy the com obj value into a telegram object
void KnxComObject::CopyValue(KnxTelegram& dest) const
{
	if (_length == 1) dest.SetFirstPayloadByte(_value);
	else if (_length == 2 )dest.SetLongPayload(&_value, 1);
	else dest.SetLongPayload(_longValue, _length - 1);
}


// DEBUG function
void KnxComObject::Info(String& str) const
{
byte length = GetLength();
	str+="Addr=" + String(GetAddr(),HEX);
	str+="\nDPTId=" + String(GetDptId(),HEX);
	str+="\nIndicator=" + String(GetIndicator(),HEX);
	str+="\nLength=" + String(length,DEC);
        str+="\nPrio=";
	switch(GetPriority())
	{
		case KNX_PRIORITY_SYSTEM_VALUE : str+="SYSTEM"; break;
		case KNX_PRIORITY_ALARM_VALUE : str+="ALARM"; break;
		case KNX_PRIORITY_HIGH_VALUE : str+="HIGH"; break;
		case KNX_PRIORITY_NORMAL_VALUE : str+="NORMAL"; break;
		default : str+="ERR_VAL!"; break;
	}
	str+="\nValidity="; if (GetValidity()) str+= "YES"; else str+="NO";

	if (length >2) str+="\nShortValue=N/A";
	else str+="\nShortValue=" + String(GetValue(),HEX);
	if (length <=2) str+="\nLongValue=N/A";
	else 
        {
		str+="\nLongValue=";
		byte *longValue = (byte *) malloc(length- 1);
                GetValue(longValue);
		for (byte i = 0; i < length-1; i++) str+=String(longValue[i], HEX)+' ';
	}
}

//EOF
