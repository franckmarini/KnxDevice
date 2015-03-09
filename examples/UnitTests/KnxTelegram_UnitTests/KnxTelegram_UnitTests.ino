#include <KnxDevice.h>
#include <Cli.h> // command line interpreter lib available at https://github.com/franckmarini/Cli

Cli cli = Cli(Serial);

void PriorityTests(void);
void RepeatTests(void);
void AddressesTests(void);
void CounterTests(void);
void LengthTests(void);
void CommandTests(void);
void PayloadTests(void);
void ChecksumTests(void);
void CopyTests(void);
void AllTests(void);


void setup(){
  cli.RegisterCmd("prio",&PriorityTests);
  cli.RegisterCmd("repeat",&RepeatTests);
  cli.RegisterCmd("addr",&AddressesTests);
  cli.RegisterCmd("counter",&CounterTests);
  cli.RegisterCmd("length",&LengthTests);
  cli.RegisterCmd("cmd",&CommandTests);
  cli.RegisterCmd("pld",&PayloadTests);
  cli.RegisterCmd("cks",&ChecksumTests);
  cli.RegisterCmd("copy",&CopyTests);
  cli.RegisterCmd("all",&AllTests);
  Serial.begin(115200);
}


void loop(){
  cli.Run();
}  


void PrintTelegramInfo(KnxTelegram& tg, String& str) 
{
  str = " => Info() :\n"; tg.Info(str);
  str += "=> InfoRaw() :\n"; tg.InfoRaw(str);
  str += "=> InfoVerbose() :\n"; tg.InfoVerbose(str);
  Serial.print(str);
}


void PriorityTests(void)
{
    String traces;
    KnxTelegram telegram;
    Serial.println(F("\n########## Priority Tests ##########"));
    
    Serial.println(F("#### Telegram at construction ####"));
    PrintTelegramInfo(telegram, traces);
    Serial.println(F("#### Set Priority to SYSTEM_VALUE ####"));
    telegram.ChangePriority(KNX_PRIORITY_SYSTEM_VALUE);
    PrintTelegramInfo(telegram, traces);
    
    Serial.println(F("#### Set Priority to HIGH_VALUE ####"));
    telegram.ChangePriority(KNX_PRIORITY_HIGH_VALUE);
    PrintTelegramInfo(telegram, traces);

    Serial.println(F("#### Set Priority to ALARM_VALUE ####"));
    telegram.ChangePriority(KNX_PRIORITY_ALARM_VALUE);
    PrintTelegramInfo(telegram, traces);

    Serial.println(F("#### Set Priority to NORMAL_VALUE ####"));
    telegram.ChangePriority(KNX_PRIORITY_NORMAL_VALUE);
    PrintTelegramInfo(telegram, traces);
}


void RepeatTests(void)
{
    String traces;
    KnxTelegram telegram;
    Serial.println(F("\n########## Repeat Tests ##########"));
    
    Serial.println(F("#### Telegram at construction ####"));
    PrintTelegramInfo(telegram, traces);
    
    Serial.println(F("#### SetRepeated ####"));
    telegram.SetRepeated();
    PrintTelegramInfo(telegram, traces);
}


void AddressesTests(void)
{
    String traces;
    KnxTelegram telegram;
    Serial.println(F("\n########## Adresses Tests ##########"));

    Serial.println(F("#### Telegram at construction ####"));
    PrintTelegramInfo(telegram, traces);

    Serial.println(F("#### SetSourceAddress 0x1234 ####"));
    telegram.SetSourceAddress(0x1234);
    PrintTelegramInfo(telegram, traces);
    
    Serial.println(F("#### SetTargetAddress 0x5678 ####"));
    telegram.SetTargetAddress(0x5678);
    PrintTelegramInfo(telegram, traces);
    
    Serial.println(F("#### Set Unicast ####"));
    telegram.SetMulticast(false);
    PrintTelegramInfo(telegram, traces);
    
    Serial.println(F("#### Set Multicast ####"));
    telegram.SetMulticast(true);
    PrintTelegramInfo(telegram, traces);    
}


void CounterTests(void)
{
    String traces;
    KnxTelegram telegram;
    Serial.println(F("\n########## Routing Counter Tests ##########"));

    Serial.println(F("#### Telegram at construction ####"));
    PrintTelegramInfo(telegram, traces);

    Serial.println(F("#### ChangeRoutingCounter to 4 ####"));
    telegram.ChangeRoutingCounter(4);
    PrintTelegramInfo(telegram, traces);
}


void LengthTests(void)
{
    String traces;
    KnxTelegram telegram;
    Serial.println(F("\n########## Payload Length Tests ##########"));  

    Serial.println(F("#### Telegram at construction ####"));
    PrintTelegramInfo(telegram, traces);      

    Serial.println(F("#### Set Payload Length to 2 ####"));
    telegram.SetPayloadLength(2);
    PrintTelegramInfo(telegram, traces);
}


void CommandTests(void)
{
    String traces;
    KnxTelegram telegram;
    Serial.println(F("\n########## Command Tests ##########"));  

    Serial.println(F("#### Telegram at construction ####"));
    PrintTelegramInfo(telegram, traces);
   
    Serial.println(F("#### Set VALUE_RESPONSE Command ####"));
    telegram.SetCommand(KNX_COMMAND_VALUE_RESPONSE);
    PrintTelegramInfo(telegram, traces);
   
    Serial.println(F("#### Set VALUE_WRITE Command ####"));
    telegram.SetCommand(KNX_COMMAND_VALUE_WRITE);
    PrintTelegramInfo(telegram, traces);
   
    Serial.println(F("#### Set MEMORY_WRITE Command ####"));
    telegram.SetCommand(KNX_COMMAND_MEMORY_WRITE);
    PrintTelegramInfo(telegram, traces);

    Serial.println(F("#### Set VALUE_READ Command ####"));
    telegram.SetCommand(KNX_COMMAND_VALUE_READ);
    PrintTelegramInfo(telegram, traces);
}


void PayloadTests(void)
{
    String traces;
    KnxTelegram telegram;
    Serial.println(F("\n########## Payload Tests ##########"));  

    Serial.println(F("#### Telegram at construction ####"));
    PrintTelegramInfo(telegram, traces);

    Serial.println(F("#### Set First Payload Byte to 8 ####"));
    telegram.SetFirstPayloadByte(0x8);
    PrintTelegramInfo(telegram, traces);
    
    Serial.println(F("#### Clear First Payload Byte ####"));
    telegram.ClearFirstPayloadByte();
    PrintTelegramInfo(telegram, traces);
    
    Serial.println(F("#### Set First Payload Byte to the truncated value 0xFF ####"));
    telegram.SetFirstPayloadByte(0xFF);
    PrintTelegramInfo(telegram, traces);
        
    Serial.println(F("#### SetLongPayload 'a','b','c','d','e','f','g','h','i','j','k','l','m','n' ####"));
    byte payload1[14]={'a','b','c','d','e','f','g','h','i','j','k','l','m','n'};
    telegram.SetLongPayload(payload1, 14);
    telegram.SetPayloadLength(15);
    PrintTelegramInfo(telegram, traces);

    Serial.println(F("#### GetLongPayload() ####"));    
    byte payload2[14];
    telegram.GetLongPayload(payload2,14);
    for (byte i = 0;i < 14; i++) {Serial.print(payload2[i],HEX); ; Serial.print(' ');}
    Serial.println();

    Serial.println(F("#### ClearLongPayload() ####"));
    telegram.ClearLongPayload();
    PrintTelegramInfo(telegram, traces);
}


void ChecksumTests(void)
{
    String traces;
    KnxTelegram telegram;
    Serial.println(F("\n########## Checksum Tests ##########"));  

    Serial.println(F("#### Telegram at construction ####"));
    PrintTelegramInfo(telegram, traces);
    
    Serial.println(F("#### IsChecksumCorrect####"));    
    if (telegram.IsChecksumCorrect()) Serial.println("YES"); else Serial.println("NO");
    
    Serial.println(F("#### UpdateChecksum ####"));
    telegram.UpdateChecksum();
    PrintTelegramInfo(telegram, traces);

    Serial.println(F("#### IsChecksumCorrect####"));    
    if (telegram.IsChecksumCorrect()) Serial.println("YES"); else Serial.println("NO");
}


void CopyTests(void)
{
     String traces;
    KnxTelegram origin, destination;
    Serial.println(F("\n########## Copy Tests ##########"));  
    
    origin.SetRepeated();
    origin.SetSourceAddress(0x1234);
    origin.SetTargetAddress(0x5678);
    origin.SetFirstPayloadByte(0x4);
    byte payload1[]={'a','b','c','d','e','f','g','h','i','j','k','l','m','n'};
    origin.SetLongPayload(payload1, sizeof(payload1));
    origin.SetPayloadLength(15);
    origin.UpdateChecksum();
    Serial.println(F("#### origin telegram at construction ####"));
    PrintTelegramInfo(origin, traces);
    Serial.println(F("#### destination telegram at construction ####"));
    PrintTelegramInfo(destination, traces);

    Serial.println(F("#### Header Copy origin->destination ####"));    
    origin.CopyHeader(destination);
    Serial.println(F("#### destination telegram ####"));
    PrintTelegramInfo(destination, traces);

    Serial.println(F("#### Clear destination telegram ####"));
    destination.ClearTelegram();
    Serial.println(F("#### destination telegram ####"));
    PrintTelegramInfo(destination, traces);    

    Serial.println(F("#### Copy origin->destination ####"));    
    origin.Copy(destination);
    Serial.println(F("#### destination telegram ####"));
    PrintTelegramInfo(destination, traces);
    
    Serial.println(F("#### Clear destination telegram ####"));
    destination.ClearTelegram();
    Serial.println(F("#### destination telegram ####"));
    PrintTelegramInfo(destination, traces);   
}


void AllTests(void)
{
  PriorityTests();
  RepeatTests();
  AddressesTests();
  CounterTests();
  LengthTests();
  CommandTests();
  PayloadTests();
  ChecksumTests();
  CopyTests();
}
