# KNX Bus Device library for Arduino

## Links :
- [Blog](http://www.liwan.fr/KnxWithArduino/)
- [GitHub Page](http://franckmarini.github.io/KnxDevice)
- [KNX Association](http://www.knx.org)
- [Siemens KNX chipsets](http://www.buildingtechnologies.siemens.com/bt/global/en/buildingautomation-hvac/gamma-building-control/gamma-b2b/Pages/transceivers.aspx)

## Realization examples :
- Prototype of Basic push button device  (watch the demo in the [Blog](http://www.liwan.fr/KnxWithArduino/))
- Prototype of Green Light actuator device 

NB : The source code is available in the "examples" folder.

## Presentation :
KNX is an open communication protocol standard for intelligent buildings.

This library allows you to create your "self-made" KNX bus device.
For that, you need an arduino hardware and a Siemens TPUART chipset for the physical coupling to the KNX bus (see hardware section below).
To avoid spending energy on electronic stuff, the easiest way (I chose) is to use an electronic board with the TPUART already integrated : I used a "TPUART2 test Board BTM2-PCB" that I bought from http://www.opternus.com. Or a Siemens bus coupler should also be OK even if I have not tested it. Or why not create a new PCB with both Arduino and TPUART integrated (any motivated person?).

You also need to know a few things about the KNX system, in particular about KNX communication.
There are plenty of information on the web, or you can also read the "KNX Basic Course Documentation" book, available on the knx online shop (www.knx.org), which offers a complete technical overview of the KNX system.

Why to create its own KNX devices ? First this library is intended for hobbyists only. It allows you to create something funny and fully customized. The main drawback is that your self-made device can not be configured using ETS, the KNX software allowing KNX installation commissionning. I hope to make this library as reliable as possible (you can help me in this task!) even if **its use remains at your own risks.** I'm still confident enough and plan to use self-made bus devices in my future own KNX home installation.


## Hardware :
For hardware part, I considered the following points : 
- The TPUART will be connected to the serial port of the Arduino.
- The TPUART delivers a stabilized 5V supply, TPUART generation1 provides up to 10mA whereas TPUART gen2 provides up to 50mA.
- The bus device (TPUART board, arduino, plus extra electronic parts) should ideally fit into a flush mounted wall box.
- The bus device shall be powered by the TPUART supply (no use of external supply)

The ideal arduino board seems to be Arduino Mini for its tight dimensions and low power consumption, around 10mA with power optimization.
But its drawback is the presense of one serial only, meaning you cannot debug while the bus device is running.

That's why, for the development of the software library, I have used the Arduino Mega offering several serials : Serial0 is used for programming & debug, while Serial1 is connected to the TPUART. Since the Arduino Mega is connected and powered by the USB port, I isolated the RX/TX lines between Arduino and TPUART using opto-couplers.


## Roadmap :
This library is still under developpement. The next actions in the pipe are :
- create a blog to better demonstrate examples and new device realizations, and share ideas
- create a version with reduced power consumption 
- add large com objects support (the API is actually already implemented, but it has to be tested..)
- background task : increase software maturity and reliability

## API
### 1/ Define the communication objects
First of all, define the KNX communication objects of your bus device. For each object, define its group address its gets linked to, its datapoint type, and its flags. Theoritically, you can define up to 256 objects, even if in practical you are limited by the quantity of RAM (it would be worth measuring the max allowed number of objects depending on the memory available).

**`KnxComObject KnxDevice::_comObjectsList[];`**

* **Description:** list of the communication objects (group objects) that are attached to your KNX device. Define this variable in your Arduino sketch (but outside all function bodies).
* **Parameters:** for each object in the list, you shall provide the group address (word, use G_ADDR() function), the datapoint type (check "_e_KnxDPT_ID_" enum in [KnxDPT.h](https://github.com/franckmarini/KnxDevice/blob/master/KnxDPT.h) file), and the flags (byte, check [KnxComObject.h](https://github.com/franckmarini/KnxDevice/blob/master/KnxComObject.h) for more details). 
* **Example:** 
```
// Definition of the Communication Objects attached to the device
KnxComObject KnxDevice::_comObjectsList[] =
{
//             	adress,			                         DataPoint ID,						                flags			} ,
/* Index 0  */ { G_ADDR(0,0,1) /* addr 0.0.1 */,		  KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ ,	          COM_OBJ_LOGIC_IN_INIT	} ,
/* Index 1  */ { G_ADDR(0,0,2) /* addr 0.0.2 */,		  KNX_DPT_5_010 /* 5.010 U8 DPT_Value_1_Ucount */ ,	  COM_OBJ_SENSOR		} ,
/* Index 2  */ { G_ADDR(0,0,0xA) /* addr 0.0.A */,        KNX_DPT_1_003 /* 1.003 B1 DPT_Enable*/ ,		      0x30 /* C+R */		} ,
};
```
___
**`const byte KnxDevice::_comObjectsNb = sizeof(_comObjectsList) / sizeof(KnxComObject);`**
* **Description:** Define the number of group objects in the list. Simply copy the above code as is in your Arduino sketch!

### 2/ Start/Stop/Run the KNX device
___
**`byte begin(HardwareSerial& serial, word physicalAddr);`**
* **Description:**  Start the KNX Device. Place this function call in the setup() function of your Arduino sketch
* **Parameters :** "serial" is the Hardware serial port connected to the TPUART. "physicalAddr" is the physical address of your device (use P_ADDR() function).
* **Return value :** return KNX_DEVICE_ERROR (255) if begin() failed, else return KNX_DEVICE_OK
* **Example:** 
```
Knx.begin(Serial, P_ADDR(1,1,0xAA)); // start a KnxDevice session with physical address "1.1.AA" on "Serial" UART
```

___
**`void task(void);`**
* **Description:**  KNX device execution task. This function call shall be placed in the "loop()" Arduino function. **WARNING : this function shall be called periodically (400us max period) meaning usage of functions stopping the execution (like delay(), visit http://playground.arduino.cc/Code/AvoidDelay for more info) is FORBIDDEN.**
* **Example:** 
```
Knx.task();
```
___
**`void end(void);`**
* **Description:**  Stop the KNX Device. This function usage should be unusual.
* **Example:** 
```
Knx.end();
```
___
### 3/ Interact with the communication objects
The API allows you to interact with objects that you have defined : you can read and modify their values, force their value to be updated with the value on the bus. You are also notified each time objects get their value changed following a bus access :
___
**`void knxEvents(byte objectIndex);`**

* **Description:**  callback function that is called by the KnxDevice library every time a group object is updated by the bus. Define this function in your Arduino sketch.
* **Parameters :** "objectIndex" is the index (in the list) of the object updated by the bus
* **Example:**
```
// Callback function to treat object updates
void knxEvents(byte index) {
  switch (index)
  {
    case 0 : // object index 0 has been updated
      // code to treat index 0 object update
      break;

    case 1 : // object index 0 has been updaed
      // code to treat index 1 object update
      break;

//  ...

    default:
      // code to treat remaining objects updates
      break;
  }
};
```

___
**`byte Knx.read(byte objectIndex);`**

* **Description:** Get the current value of a group object. This function is relevant for _short_ objects only.
* **Parameters:** "objectIndex" is the index (in the list) of the object to be read.
* **Return:** the current value of the object.
* **Example:** ```Knx.read(0); // return the value of the 1st object in the list```

___
**`void Knx.write(byte objectIndex, byte byteValue);`**

* **Description:** update the value of a group object. This function is relevant for _short_ objects only (the function has no effect on _long_ objects)
In case the object has communication and transmit flags set, then a telegram is emitted on the EIB bus, thus the new value is propagated to the other devices.
* **Parameters:** "objectIndex" is the index (in the list) of the object to be updated. "byteValue" is the new value.
* **Example:** ```Knx.write(0,1); // the object with index 0 gets value 1.```

___
**`void Knx.update(byte objectIndex);`**

* **Description:** request the group object to be updated with the value from the bus. Note that this function is _asynchroneous_, the update completion is notified by the knxEvents() callback. This function is relevant only for objects with 
* **Parameters:** "objectIndex" is the index (in the list) of the object to be updated. 
* **Example:** ```Knx.update(0); // request the update of the object with index 0.```

___

