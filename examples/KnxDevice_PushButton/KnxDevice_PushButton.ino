// KNX toggle push button using KnxDevice library
// The push button drives one channel of a switch actuator device.
// The feedback status of the channel is considered : in this way, the toggle value is always correct and the actuator might be driven by several buttons.

// Required environment :
// - a KNX network with a TPUART board and a switch actuator device
// - ComObject 0.0.1 switches one of the actuator channel (configuration performed with ETS)
// - ComObject 0.0.2 is the feedback status of the channel  (configuration performed with ETS)
// - Arduino Mini Pro with its serial port connected to HW TPUART interface
// - a push button conected to digital port 2

#include <KnxDevice.h>

// Definition of the Communication Objects attached to the device
KnxComObject KnxDevice::_comObjectsList[] =
{
  /* Index 0 */ KnxComObject(G_ADDR(0,0,1), KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_SENSOR  /* Sensor Output */ ) ,
  /* Index 1 */ KnxComObject(G_ADDR(0,0,2), KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN_INIT /* Logical Input Object with Init Read */) , 
};

const byte KnxDevice::_comObjectsNb = sizeof(_comObjectsList) / sizeof(KnxComObject); // do no change this code

// function and variables to manage push button signal debounce
static inline word TimeDeltaWord(word now, word before) { return (word)(now - before); }

boolean pressed = false, debounce = false;
word debounceStartTime;


// Callback function to handle com objects updates
void knxEvents(byte index) {}


void setup(){
  attachInterrupt(0 /* int. 0 on pin 2 */, ButtonReleased, FALLING);
  Knx.begin(Serial, P_ADDR(1,1,1)); // start a KnxDevice session with physical address 1.1.1 on Serial UART
}


void loop(){ 
  Knx.task();
  if (debounce && (TimeDeltaWord((word)millis(), debounceStartTime)> 500 /* ms */)) debounce = false;
  if (pressed) { pressed=false; Knx.write(0,!Knx.read(1)); }
}



void ButtonReleased()
{
  if(!debounce) { pressed = true; debounce = true; debounceStartTime = (word) millis();}
}

