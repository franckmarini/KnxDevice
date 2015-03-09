
// Sketch allowing KNX Library debugging using CLI library and an ARDUINO MEGA board.
// TPUART interface is connected to Serial1 using opto-couplers.

// here we simulate a push button and a KNX device reading 2 luminosity com objects.

#include <Cli.h> // https://github.com/franckmarini/Cli
#include <KnxDevice.h>

Cli cli = Cli(Serial);

void SimulateButtonPush(void) {Serial.println("push"); Knx.write(0, (byte)!Knx.read(1)); }
void ReadLuminosity1(void) { Knx.update(2);}
void ReadLuminosity2(void) { Knx.update(3);}

// Definition of the Communication Objects attached to the device
KnxComObject KnxDevice::_comObjectsList[] =
{
  /* Index 0 */ KnxComObject(G_ADDR(2,0,13), KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_SENSOR) ,
  /* Index 1 */ KnxComObject(G_ADDR(2,1,13), KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN_INIT) , // Logical Input Object
  /* Index 2 */ KnxComObject(G_ADDR(3,0,0), KNX_DPT_9_004 /* 9.004 F16 DPT_Value_Lux */ , COM_OBJ_LOGIC_IN) , // Logical Input Object
  /* Index 3 */ KnxComObject(G_ADDR(3,0,1), KNX_DPT_9_004 /* 9.004 F16 DPT_Value_Lux */ , COM_OBJ_LOGIC_IN) , // Logical Input Object
};

const byte KnxDevice::_comObjectsNb = sizeof(_comObjectsList) / sizeof(KnxComObject); // do no change this code

float luminosity;

// Callback function to handle com objects updates
void knxEvents(byte index) {
 if (index==2)
 {
   Knx.read(2,luminosity); Serial.print("lum1="); Serial.println(luminosity);
 }
 else if (index==3)
 {
   Knx.read(3,luminosity); Serial.print("lum2="); Serial.println(luminosity); 
 }
}


void setup(){
  cli.RegisterCmd("p",&SimulateButtonPush);
  cli.RegisterCmd("l1",&ReadLuminosity1);
  cli.RegisterCmd("l2",&ReadLuminosity2);
  Serial.begin(115200);
  if (Knx.begin(Serial1, P_ADDR(1,1,1)) == KNX_DEVICE_ERROR) {Serial.println("knx init ERROR, stop here!!");while(1);}
  Serial.println("knx started...");
}


void loop(){ 
  Knx.task();
  cli.Run();
}







