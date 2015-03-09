// Green Light Actuator using KnxDevice library
// 
// Required environment :
// - a KNX network with a TPUART board and a push button device 
// - Configure one of the push button channel in TOGGLE mode on EIB address 0.0.2 in ETS
// - Arduino Mini Pro with its serial port connected to HW TPUART interface
// - green and red leds plus pull-up resistors connected to digital ports 7&9

#include <KnxDevice.h>

int greenPin = 7; // green LED connected to pin 7 with active high
int RedPin = 9; // red LED connected to pin 9 with active high

// Definition of the Communication Objects attached to the device
KnxComObject KnxDevice::_comObjectsList[] =
{
/* Index 0 : */ KnxComObject(G_ADDR(0,0,2), KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN_INIT /* Logical Input Object with Init Read */ ) ,
};

const byte KnxDevice::_comObjectsNb = sizeof(_comObjectsList) / sizeof(KnxComObject); // do no change this code

// Callback function to handle com objects updates
void knxEvents(byte index) {
  switch (index)
  {
    case 0 : // object index 0 has been updaed
      if (Knx.read(0)) { digitalWrite(greenPin, HIGH); digitalWrite(RedPin,LOW); } // green led on, // red led off, 
      else { digitalWrite(greenPin, LOW); digitalWrite(RedPin,HIGH); } // green led off, // red led on,
      break;

    default: break;      
  }
};


void setup(){
  pinMode(greenPin, OUTPUT); 
  pinMode(RedPin, OUTPUT);
  Knx.begin(Serial, P_ADDR(1,1,1)); // start a KnxDevice session with physical address 1.1.1 on Serial UART
}


void loop(){ 
  Knx.task();
}
