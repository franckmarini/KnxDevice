
// Test of the DPT Format <=> C type conversions

#include <KnxDevice.h>

void setup()
{
  Serial.begin(115200);
 
}

void loop()
{

  Serial.println(F("\n******************************************************************"));
  Serial.println(F("*********** TEST INTEGER CONVERSIONS : DPT => C TYPES ************"));
  Serial.println(F("******************************************************************"));
  
  const byte dptOriginValue[4] = { 0xAA,0xBB, 0xCC, 0xDD };
  unsigned int resultValueUint;
  int resultValueInt;
  unsigned long resultValueUlLong;
  long resultValueLong;
  
  Serial.println(F("***********DPT U16 => UINT ***********"));
  ConvertFromDpt(dptOriginValue,resultValueUint, KNX_DPT_FORMAT_U16);
  Serial.print("resultValueInt(dec)=");Serial.println(resultValueUint);
  Serial.print("resultValueInt(hex)=");Serial.println(resultValueUint,HEX);

  Serial.println(F("\n***********DPT U16 => INT (tolerated) ***********"));
  ConvertFromDpt(dptOriginValue,resultValueInt, KNX_DPT_FORMAT_U16);
  Serial.print("resultValueInt(dec)=");Serial.println(resultValueInt);
  Serial.print("resultValueInt(hex)=");Serial.println(resultValueInt,HEX);

  Serial.println(F("\n***********DPT V16 => INT ***********"));
  ConvertFromDpt(dptOriginValue,resultValueInt, KNX_DPT_FORMAT_V16);
  Serial.print("resultValueInt(dec)=");Serial.println(resultValueInt);
  Serial.print("resultValueInt(hex)=");Serial.println(resultValueInt,HEX);

  Serial.println(F("\n***********DPT V16 => UINT (tolerated) ***********"));
  ConvertFromDpt(dptOriginValue,resultValueUint, KNX_DPT_FORMAT_V16);
  Serial.print("resultValueInt(dec)=");Serial.println(resultValueUint);
  Serial.print("resultValueInt(hex)=");Serial.println(resultValueUint,HEX);

  Serial.println(F("\n***********DPT U32 => ULONG ***********"));
  ConvertFromDpt(dptOriginValue,resultValueUlLong, KNX_DPT_FORMAT_U32);
  Serial.print("resultValueInt(dec)=");Serial.println(resultValueUlLong);
  Serial.print("resultValueInt(hex)=");Serial.println(resultValueUlLong,HEX);
  
  Serial.println(F("\n***********DPT U32 => LONG (tolerated) ***********"));
  ConvertFromDpt(dptOriginValue,resultValueLong, KNX_DPT_FORMAT_U32);
  Serial.print("resultValueInt(dec)=");Serial.println(resultValueLong);
  Serial.print("resultValueInt(hex)=");Serial.println(resultValueLong,HEX);

  Serial.println(F("\n***********DPT V32 => LONG ***********"));
  ConvertFromDpt(dptOriginValue,resultValueLong, KNX_DPT_FORMAT_V32);
  Serial.print("resultValueInt(dec)=");Serial.println(resultValueLong);
  Serial.print("resultValueInt(hex)=");Serial.println(resultValueLong,HEX);
  
  Serial.println(F("\n***********DPT V32 => ULONG (tolerated) ***********"));
  ConvertFromDpt(dptOriginValue,resultValueUlLong, KNX_DPT_FORMAT_V32);
  Serial.print("resultValueInt(dec)=");Serial.println(resultValueUlLong);
  Serial.print("resultValueInt(hex)=");Serial.println(resultValueUlLong,HEX);


  Serial.println(F("\n******************************************************************"));
  Serial.println(F("*********** TEST INTEGER CONVERSIONS : C TYPES => DPT ************"));
  Serial.println(F("******************************************************************"));

  byte dptDestValue[4] = {0,0,0,0};
  unsigned int originValueUint = 0xAABB;
  int originValueInt = 0xAABB;
  unsigned long originValueUlLong = 0xAABBCCDD;
  long originValueLong = 0xAABBCCDD;

  Serial.println(F("*********** UINT => DPT U16 ***********"));
  ConvertToDpt(originValueUint, dptDestValue, KNX_DPT_FORMAT_U16);
  Serial.print("dptDestValue="); for (int i=0; i<4; i++) {   Serial.print(dptDestValue[i],HEX); Serial.print(" "); }
  for (int i=0; i<4; i++) dptDestValue[i] = 0; // reset dptDestValue;

  Serial.println(F("\n*********** INT => DPT U16 (tolerated) ***********"));
  ConvertToDpt(originValueInt, dptDestValue, KNX_DPT_FORMAT_U16);
  Serial.print("dptDestValue="); for (int i=0; i<4; i++) {   Serial.print(dptDestValue[i],HEX); Serial.print(" "); }
  for (int i=0; i<4; i++) dptDestValue[i] = 0; // reset dptDestValue;  
  
  Serial.println(F("\n*********** INT => DPT V16 ***********"));
  ConvertToDpt(originValueInt, dptDestValue, KNX_DPT_FORMAT_V16);
  Serial.print("dptDestValue="); for (int i=0; i<4; i++) {   Serial.print(dptDestValue[i],HEX); Serial.print(" "); }
  for (int i=0; i<4; i++) dptDestValue[i] = 0; // reset dptDestValue; 

  Serial.println(F("\n*********** UINT => DPT V16 (tolerated) ***********"));
  ConvertToDpt(originValueUint, dptDestValue, KNX_DPT_FORMAT_V16);
  Serial.print("dptDestValue="); for (int i=0; i<4; i++) {   Serial.print(dptDestValue[i],HEX); Serial.print(" "); }
  for (int i=0; i<4; i++) dptDestValue[i] = 0; // reset dptDestValue;   

  Serial.println(F("\n*********** ULONG => DPT U32 ***********"));
  ConvertToDpt(originValueUlLong, dptDestValue, KNX_DPT_FORMAT_U32);
  Serial.print("dptDestValue="); for (int i=0; i<4; i++) {   Serial.print(dptDestValue[i],HEX); Serial.print(" "); }
  for (int i=0; i<4; i++) dptDestValue[i] = 0; // reset dptDestValue;

  Serial.println(F("\n*********** LONG => DPT U32 (tolerated) ***********"));
  ConvertToDpt(originValueLong, dptDestValue, KNX_DPT_FORMAT_U32);
  Serial.print("dptDestValue="); for (int i=0; i<4; i++) {   Serial.print(dptDestValue[i],HEX); Serial.print(" "); }
  for (int i=0; i<4; i++) dptDestValue[i] = 0; // reset dptDestValue;  

  Serial.println(F("\n*********** LONG => DPT V32 ***********"));
  ConvertToDpt(originValueLong, dptDestValue, KNX_DPT_FORMAT_V32);
  Serial.print("dptDestValue="); for (int i=0; i<4; i++) {   Serial.print(dptDestValue[i],HEX); Serial.print(" "); }
  for (int i=0; i<4; i++) dptDestValue[i] = 0; // reset dptDestValue;

  Serial.println(F("\n*********** ULONG => DPT V32 (tolerated) ***********"));
  ConvertToDpt(originValueUlLong, dptDestValue, KNX_DPT_FORMAT_V32);
  Serial.print("dptDestValue="); for (int i=0; i<4; i++) {   Serial.print(dptDestValue[i],HEX); Serial.print(" "); }
  for (int i=0; i<4; i++) dptDestValue[i] = 0; // reset dptDestValue;  


  Serial.println(F("\n\n*********************************************************************************************"));
  Serial.println(F("*********** TEST FLOAT CONVERSIONS : float C TYPE => DPT F16 => float C TYPE  ***************"));
  Serial.println(F("*********************************************************************************************"));


  float originValueFloat;
  float destValueFloat = 0;

  originValueFloat = 1234.56;
  Serial.print("originValueFloat=");Serial.println(originValueFloat);
  ConvertToDpt(originValueFloat, dptDestValue, KNX_DPT_FORMAT_F16);
  Serial.print("dptDestValue="); for (int i=0; i<2; i++) {   Serial.print(dptDestValue[i],HEX); Serial.print(" "); }
  ConvertFromDpt(dptDestValue, destValueFloat, KNX_DPT_FORMAT_F16);
  Serial.print("\ndestValueFloat=");Serial.println(destValueFloat);
  for (int i=0; i<2; i++) dptDestValue[i] = 0; // reset dptDestValue
  destValueFloat = 0; // reset destValueFloat
  
  originValueFloat = -1234.56;
  Serial.print("originValueFloat=");Serial.println(originValueFloat);
  ConvertToDpt(originValueFloat, dptDestValue, KNX_DPT_FORMAT_F16);
  Serial.print("dptDestValue="); for (int i=0; i<2; i++) {   Serial.print(dptDestValue[i],HEX); Serial.print(" "); }
  ConvertFromDpt(dptDestValue, destValueFloat, KNX_DPT_FORMAT_F16);
  Serial.print("\ndestValueFloat=");Serial.println(destValueFloat);
  for (int i=0; i<2; i++) dptDestValue[i] = 0; // reset dptDestValue
  destValueFloat = 0; // reset destValueFloat

  originValueFloat = 0.01;
  Serial.print("originValueFloat=");Serial.println(originValueFloat);
  ConvertToDpt(originValueFloat, dptDestValue, KNX_DPT_FORMAT_F16);
  Serial.print("dptDestValue="); for (int i=0; i<2; i++) {   Serial.print(dptDestValue[i],HEX); Serial.print(" "); }
  ConvertFromDpt(dptDestValue, destValueFloat, KNX_DPT_FORMAT_F16);
  Serial.print("\ndestValueFloat=");Serial.println(destValueFloat);
  for (int i=0; i<2; i++) dptDestValue[i] = 0; // reset dptDestValue
  destValueFloat = 0; // reset destValueFloat
 
  originValueFloat = 0.0;
  Serial.print("originValueFloat=");Serial.println(originValueFloat);
  ConvertToDpt(originValueFloat, dptDestValue, KNX_DPT_FORMAT_F16);
  Serial.print("dptDestValue="); for (int i=0; i<2; i++) {   Serial.print(dptDestValue[i],HEX); Serial.print(" "); }
  ConvertFromDpt(dptDestValue, destValueFloat, KNX_DPT_FORMAT_F16);
  Serial.print("\ndestValueFloat=");Serial.println(destValueFloat);
  for (int i=0; i<2; i++) dptDestValue[i] = 0; // reset dptDestValue
  destValueFloat = 0; // reset destValueFloat
 
  originValueFloat = -0.01;
  Serial.print("originValueFloat=");Serial.println(originValueFloat);
  ConvertToDpt(originValueFloat, dptDestValue, KNX_DPT_FORMAT_F16);
  Serial.print("dptDestValue="); for (int i=0; i<2; i++) {   Serial.print(dptDestValue[i],HEX); Serial.print(" "); }
  ConvertFromDpt(dptDestValue, destValueFloat, KNX_DPT_FORMAT_F16);
  Serial.print("\ndestValueFloat=");Serial.println(destValueFloat);
  for (int i=0; i<2; i++) dptDestValue[i] = 0; // reset dptDestValue
  destValueFloat = 0; // reset destValueFloat
  
  originValueFloat = -671088.64;
  Serial.print("originValueFloat=");Serial.println(originValueFloat);
  ConvertToDpt(originValueFloat, dptDestValue, KNX_DPT_FORMAT_F16);
  Serial.print("dptDestValue="); for (int i=0; i<2; i++) {   Serial.print(dptDestValue[i],HEX); Serial.print(" "); }
  ConvertFromDpt(dptDestValue, destValueFloat, KNX_DPT_FORMAT_F16);
  Serial.print("\ndestValueFloat=");Serial.println(destValueFloat);
  for (int i=0; i<2; i++) dptDestValue[i] = 0; // reset dptDestValue
  destValueFloat = 0; // reset destValueFloat
  
  originValueFloat = 670760.96;
  Serial.print("originValueFloat=");Serial.println(originValueFloat);
  ConvertToDpt(originValueFloat, dptDestValue, KNX_DPT_FORMAT_F16);
  Serial.print("dptDestValue="); for (int i=0; i<2; i++) {   Serial.print(dptDestValue[i],HEX); Serial.print(" "); }
  ConvertFromDpt(dptDestValue, destValueFloat, KNX_DPT_FORMAT_F16);
  Serial.print("\ndestValueFloat=");Serial.println(destValueFloat);
  for (int i=0; i<2; i++) dptDestValue[i] = 0; // reset dptDestValue
  destValueFloat = 0; // reset destValueFloat


  Serial.println(F("\n\n*********************************************************************************************"));
  Serial.println(F("*********** TEST FLOAT CONVERSIONS : LONG C TYPE => DPT F16 => LONG C TYPE  ***************"));
  Serial.println(F("*********************************************************************************************"));

  long destValueLong;
  
  originValueLong = 123456;
  Serial.print("originValueLong=");Serial.println(originValueLong);
  ConvertToDpt(originValueLong, dptDestValue, KNX_DPT_FORMAT_F16);
  Serial.print("dptDestValue="); for (int i=0; i<2; i++) {   Serial.print(dptDestValue[i],HEX); Serial.print(" "); }
  ConvertFromDpt(dptDestValue, destValueLong, KNX_DPT_FORMAT_F16);
  Serial.print("\ndestValueLong=");Serial.println(destValueLong);
  for (int i=0; i<2; i++) dptDestValue[i] = 0; // reset dptDestValue
  destValueFloat = 0; // reset destValueFloat

  originValueLong = -123456;
  Serial.print("originValueLong=");Serial.println(originValueLong);
  ConvertToDpt(originValueLong, dptDestValue, KNX_DPT_FORMAT_F16);
  Serial.print("dptDestValue="); for (int i=0; i<2; i++) {   Serial.print(dptDestValue[i],HEX); Serial.print(" "); }
  ConvertFromDpt(dptDestValue, destValueLong, KNX_DPT_FORMAT_F16);
  Serial.print("\ndestValueLong=");Serial.println(destValueLong);
  for (int i=0; i<2; i++) dptDestValue[i] = 0; // reset dptDestValue
  destValueFloat = 0; // reset destValueFloat


  Serial.println(F("\n\n*********************************************************************************************"));
  Serial.println(F("*********** TEST FLOAT CONVERSIONS : INT C TYPE => DPT F16 => INT C TYPE  ***************"));
  Serial.println(F("*********************************************************************************************"));

  int destValueInt;
  originValueInt = -3456;
  Serial.print("originValueInt=");Serial.println(originValueInt);
  ConvertToDpt(originValueInt, dptDestValue, KNX_DPT_FORMAT_F16);
  Serial.print("dptDestValue="); for (int i=0; i<2; i++) {   Serial.print(dptDestValue[i],HEX); Serial.print(" "); }
  ConvertFromDpt(dptDestValue, destValueInt, KNX_DPT_FORMAT_F16);
  Serial.print("\ndestValueInt=");Serial.println(destValueInt);
  for (int i=0; i<2; i++) dptDestValue[i] = 0; // reset dptDestValue
  destValueFloat = 0; // reset destValueFloat

while(1); // Stop here
}

