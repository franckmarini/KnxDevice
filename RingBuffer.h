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


// File : RingBuffer.h
// Author : Franck Marini
// Description : Implementation of a generic elements ring buffer
// Module dependencies : none

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include "Arduino.h"

// !!!!!!!!!!!!!!! FLAG OPTIONS !!!!!!!!!!!!!!!!!
// #define RINGBUFFER_STAT // To be uncommented when doing Statistics


// The type of the contained elements and the ring buffer size are defined at compile time (template)
// In case of buffer full, a new appended data overwrites the oldest one

template<typename T, word size>
class RingBuffer {
     byte _head;
     byte _tail;
     T _buffer[size]; // elements buffer
     byte _size;
     byte _elementsCurrentNb;
#ifdef RINGBUFFER_STAT
     byte _elementsMaxNb;
     word _lostElementsNb;
#endif

  public : 

    // Constructor
    RingBuffer()
    {
      _head = 0;
      _tail = 0;
      _elementsCurrentNb = 0;
      _size = size;
    #ifdef RINGBUFFER_STAT
      _elementsMaxNb = 0; // MAX nb of elements
      _lostElementsNb = 0;    // nb of lost elements
    #endif
    };


    // Append a data in the buffer. Append() increments the "tail"
    // In case of buffer full, a new appended data overwrites the oldest one
    void Append(const T& appendedData)
    {
      if (_elementsCurrentNb == _size)
      { // buffer is already full, we overwrite the oldest data
        IncrementHead();
    #ifdef RINGBUFFER_STAT
        _lostElementsNb++;
    #endif
      }
      else
      { // we still have some free place
        _elementsCurrentNb++;
    #ifdef RINGBUFFER_STAT
        if (_elementsCurrentNb > _elementsMaxNb) _elementsMaxNb++;
    #endif
      }
      _buffer[_tail] = appendedData;
      IncrementTail();
    }


    // Pop a data from the buffer. Pop() increments the "head"
    // Return TRUE when a data is available, otherwise FALSE
    boolean Pop(T& popData)
    {
      if (!_elementsCurrentNb) return false; // no data in the buffer
      popData = _buffer[_head];
      IncrementHead();
      _elementsCurrentNb--;
      return true;
    }


    // Return the current number of data elements in the ring buffer
    byte ElementsNb(void) const { return _elementsCurrentNb; }


    #ifdef RINGBUFFER_STAT
    // Return Stat information
    void Info(String& str)
    {
      str += "Elements Current Nb : " + String(_elementsCurrentNb,DEC);
      str += "\nElements Max Nb : " + String(_elementsMaxNb,DEC);
      str += "\nLost Elements Nb : " + String(_lostElementsNb,DEC);
      str += "\n";
    }
    #endif

  private :

    void IncrementHead(void) { _head = (_head + 1) % _size; }
    void IncrementTail(void) { _tail = (_tail + 1) % _size; }
};

#endif // RINGBUFFER_H
