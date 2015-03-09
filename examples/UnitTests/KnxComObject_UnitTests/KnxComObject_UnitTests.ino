#include <KnxDevice.h>
#include <Cli.h> // command line interpreter lib available at https://github.com/franckmarini/Cli

Cli cli = Cli(Serial);

void KNX_DPT_1_001_Tests(void); // 1 bit COM OBJ tests (1.001 B1 DPT_Switch)
void KNX_DPT_4_001_Tests(void); // 1 byte COM OBJ tests (4.001 A8 DPT_Char_ASCII)
void KNX_DPT_7_001_Tests(void); // 2 bytes COM OBJ tests (7.001 U16 DPT_Value_2_Ucount)
void AllTests(void);


void setup(){
  cli.RegisterCmd("dpt1",&KNX_DPT_1_001_Tests);
  cli.RegisterCmd("dpt4",&KNX_DPT_4_001_Tests);
  cli.RegisterCmd("dpt7",&KNX_DPT_7_001_Tests);
  cli.RegisterCmd("all",&AllTests);
  Serial.begin(115200);
}


void loop(){
  cli.Run();
}  


void PrintComObjInfo(KnxComObject& obj, String& str) 
{
  str = " => Info() :\n"; obj.Info(str);
  Serial.print(str);
}


void PrintTelegramInfo(KnxTelegram& tg, String& str) 
{
  str = " => Info() :\n"; tg.Info(str);
  str += "=> InfoRaw() :\n"; tg.InfoRaw(str);
  str += "=> InfoVerbose() :\n"; tg.InfoVerbose(str);
  Serial.print(str);
}


// 1 bit COM OBJ tests (1.001 B1 DPT_Switch)
void KNX_DPT_1_001_Tests(void)
{
    String traces;
    Serial.println(F("\n########## KNX_DPT_1_001 Tests ##########"));
    
    Serial.println(F("\n### KNX_DPT_1_001 SENSOR object creation :"));
    KnxComObject sensor(0x1234,KNX_DPT_1_001,COM_OBJ_SENSOR);
    PrintComObjInfo(sensor, traces);
    
    Serial.println(F("\n### KNX_DPT_1_001 LOGIC INPUT object creation :"));
    KnxComObject logicIn(0x1234,KNX_DPT_1_001,COM_OBJ_LOGIC_IN);
    PrintComObjInfo(logicIn, traces);
    
    Serial.println(F("\n### KNX_DPT_1_001 LOGIC INPUT WITH INIT object creation :"));
    KnxComObject logicInInit(0x1234,KNX_DPT_1_001,COM_OBJ_LOGIC_IN_INIT);
    PrintComObjInfo(logicInInit, traces);
    Serial.println(F("\n### KNX_DPT_1_001 LOGIC INPUT WITH INIT object toggle value :"));
    logicInInit.ToggleValue();
    PrintComObjInfo(logicInInit, traces);
    Serial.println(F("\n### KNX_DPT_1_001 LOGIC INPUT WITH INIT object update value (1) :"));
    logicInInit.UpdateValue(1);    
    PrintComObjInfo(logicInInit, traces);  
    Serial.println(F("\n### KNX_DPT_1_001 LOGIC INPUT WITH INIT object copy attributes into a telegram :"));
    KnxTelegram tg;
    logicInInit.CopyAttributes(tg);
    PrintTelegramInfo(tg, traces);
    Serial.println(F("\n### KNX_DPT_1_001 LOGIC INPUT WITH INIT object copy value into a telegram :"));
    logicInInit.CopyValue(tg);
    PrintTelegramInfo(tg, traces);
    Serial.println(F("\n### KNX_DPT_1_001 LOGIC INPUT WITH INIT get value from telegram (0) :"));
    tg.SetFirstPayloadByte(0);
    logicInInit.UpdateValue(tg);
    PrintComObjInfo(logicInInit, traces); 
}


// 1 byte COM OBJ tests (4.001 A8 DPT_Char_ASCII)
void KNX_DPT_4_001_Tests(void)
{
    String traces;
    Serial.println(F("\n########## KNX_DPT_4_001 Tests ##########"));
    
    Serial.println(F("\n### KNX_DPT_4_001 SENSOR object creation :"));
    KnxComObject sensor(0x1234,KNX_DPT_4_001,COM_OBJ_SENSOR);
    PrintComObjInfo(sensor, traces);
    
    Serial.println(F("\n### KNX_DPT_1_001 LOGIC INPUT object creation :"));
    KnxComObject logicIn(0x1234,KNX_DPT_4_001,COM_OBJ_LOGIC_IN);
    PrintComObjInfo(logicIn, traces);
    
    Serial.println(F("\n### KNX_DPT_4_001 LOGIC INPUT WITH INIT object creation :"));
    KnxComObject logicInInit(0x1234,KNX_DPT_4_001,COM_OBJ_LOGIC_IN_INIT);
    PrintComObjInfo(logicInInit, traces);

    Serial.println(F("\n### KNX_DPT_4_001 LOGIC INPUT WITH INIT object update value (0xAB) :"));
    logicInInit.UpdateValue(0xAB);    
    PrintComObjInfo(logicInInit, traces);  

    Serial.println(F("\n### KNX_DPT_4_001 LOGIC INPUT WITH INIT object copy attributes into a telegram :"));
    KnxTelegram tg;
    logicInInit.CopyAttributes(tg);
    PrintTelegramInfo(tg, traces);
    Serial.println(F("\n### KNX_DPT_4_001 LOGIC INPUT WITH INIT object copy value into a telegram :"));
    logicInInit.CopyValue(tg);
    PrintTelegramInfo(tg, traces);
    Serial.println(F("\n### KNX_DPT_4_001 LOGIC INPUT WITH INIT get value from telegram (0x12) :"));
    byte val = 0x12;
    tg.SetLongPayload(&val,1);
    logicInInit.UpdateValue(tg);
    PrintComObjInfo(logicInInit, traces); 
}


// 2 bytes COM OBJ tests (7.001 U16 DPT_Value_2_Ucount)
void KNX_DPT_7_001_Tests(void)
{
    String traces;
    Serial.println(F("\n########## KNX_DPT_7_001 Tests ##########"));
    
    Serial.println(F("\n### KNX_DPT_7_001 SENSOR object creation :"));
    KnxComObject sensor(0x1234,KNX_DPT_7_001,COM_OBJ_SENSOR);
    PrintComObjInfo(sensor, traces);
    
    Serial.println(F("\n### KNX_DPT_7_001 LOGIC INPUT object creation :"));
    KnxComObject logicIn(0x1234,KNX_DPT_7_001,COM_OBJ_LOGIC_IN);
    PrintComObjInfo(logicIn, traces);
    
    Serial.println(F("\n### KNX_DPT_7_001 LOGIC INPUT WITH INIT object creation :"));
    KnxComObject logicInInit(0x1234,KNX_DPT_7_001,COM_OBJ_LOGIC_IN_INIT);
    PrintComObjInfo(logicInInit, traces);

    Serial.println(F("\n### KNX_DPT_7_001 LOGIC INPUT WITH INIT object update value (0xABCD) :"));
    byte val1[2] = { 0XAB, 0xCD };
    logicInInit.UpdateValue(val1);    
    PrintComObjInfo(logicInInit, traces);  

    Serial.println(F("\n### KNX_DPT_7_001 LOGIC INPUT WITH INIT object copy attributes into a telegram :"));
    KnxTelegram tg;
    logicInInit.CopyAttributes(tg);
    PrintTelegramInfo(tg, traces);
    Serial.println(F("\n### KNX_DPT_7_001 LOGIC INPUT WITH INIT object copy value into a telegram :"));
    logicInInit.CopyValue(tg);
    PrintTelegramInfo(tg, traces);
    Serial.println(F("\n### KNX_DPT_7_001 LOGIC INPUT WITH INIT get value from telegram (0xAABB) :"));
    byte val2[2] = { 0xAA, 0xBB };
    tg.SetLongPayload(val2,2);
    logicInInit.UpdateValue(tg);
    PrintComObjInfo(logicInInit, traces); 
}


void AllTests(void)
{
KNX_DPT_1_001_Tests();
KNX_DPT_4_001_Tests();
KNX_DPT_7_001_Tests();
}

