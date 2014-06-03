// Testing Environment :
// /!\ Tests performed using a MEGA board (2 UARTs required : Serial for CLI/traces, Serial1 for TPUART)
// the Arduino UART1 is connected to a TP-UART 2 Evaluation Board
// A push button device is connected to the KNX bus :
//  - one of its channels is configured on EIB address 0x0001
//  - one of its channels is configured on EIB address 0x0003
// A switch actuator device is connected to the KNX bus :
//  - one of its channels is configured on EIB address 0x0001
//  - the feedback status of the channel is configured on EIB address 0x0002

#include <KnxDevice.h>
// NB 1 : "KNXTPUART_DEBUG_INFO" and "KNXTPUART_DEBUG_ERROR" flags shall be set in order to get KnxTpUart traces
// NB 2 : IsAddressAssigned() function shall be made public in KnxTpUart class (in KnxTpUart.h file) for Attach_Tests() test
#include <Cli.h> // command line interpreter lib available at https://github.com/franckmarini/Cli

Cli cli = Cli(Serial);
String traces;
KnxComObject objList[] =
{
  KnxComObject(0x0001, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) , // Switch actuator command
  KnxComObject(0x0002, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) , // feedback status of the switched actuator
  KnxComObject(0x0003, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) , // Push button command
};
byte resetEvt, newTgEvt, newStateEvt, ackEvt;
e_TpUartTxAck ackVal;


// WARNING : "KNXTPUART_DEBUG_INFO" and "KNXTPUART_DEBUG_ERROR" flags shall be set in order to get KnxTpUart traces
void TracesDisplay() { Serial.print(traces); traces=""; }


void PrintTelegramInfo(KnxTelegram& tg) { traces = " => Info() :\n"; tg.Info(traces); TracesDisplay(); }


boolean Pulse400us(void)
{
static word before = (word) micros();
word now = (word) micros();

  if ((word)(now -before) > 400)
  {
    before = now;
    return true;
  }
  else return false;
}


boolean Pulse800us(void)
{
static word before = (word) micros();
word now = (word) micros();

  if ((word)(now -before) > 800)
  {
    before = now;
    return true;
  }
  else return false;
}


void eventCallback(e_KnxTpUartEvent evt)
{
  if ( evt == TPUART_EVENT_RESET) resetEvt++;
  if ( evt == TPUART_EVENT_RECEIVED_EIB_TELEGRAM) newTgEvt++;
  if ( evt == TPUART_EVENT_STATE_INDICATION) newStateEvt++;
}


void ackCallback(e_TpUartTxAck ack) 
{
  ackEvt++;
  ackVal = ack;
}


void Reset_Tests(void);         // Test Reset function
void Attach_Tests(void);        // Attach Tests
void Init_Tests(void);          // Test Init function
void Bus_Monitoring(void);      // Test Bus Monitoring mode
void Normal_Rx(void);           // Test addressed telegrams reception
void Normal_Rx_ResetEvt(void);  // Test Reset Event reception
void Normal_Rx_StateEvt(void);  // Test State Event reception
void Normal_Tx_Val0(void);      // Test telegram transmission : Send boolean value 1 to valid address 0x0001 (switch actuator OFF)
void Normal_Tx_Val1(void);      // Test telegram transmission : Send boolean value 0 to valid address 0x0001 (switch actuator ON)
void Normal_Tx_NoAck(void);     // Test telegram transmission with NoAck (target address does not exist)
void Normal_Tx_Timeout(void);   // Test telegram transmission with no answer timeout (no asnwer got from the TPUART)
void Normal_Tx_Reset(void);     // Test telegram transmission with reset response
void All_Tests(void);


void setup(){
  cli.RegisterCmd("reset",&Reset_Tests);
  cli.RegisterCmd("attach",&Attach_Tests);
  cli.RegisterCmd("init",&Init_Tests);
  cli.RegisterCmd("moni",&Bus_Monitoring);
  cli.RegisterCmd("rx",&Normal_Rx);
  cli.RegisterCmd("rxreset",&Normal_Rx_ResetEvt);
  cli.RegisterCmd("rxstate",&Normal_Rx_StateEvt);
  cli.RegisterCmd("tx0",&Normal_Tx_Val0);  
  cli.RegisterCmd("tx1",&Normal_Tx_Val1);
  cli.RegisterCmd("txnoack",&Normal_Tx_NoAck);
  cli.RegisterCmd("txnoans",&Normal_Tx_Timeout);
  cli.RegisterCmd("txreset",&Normal_Tx_Reset);
  cli.RegisterCmd("all",&All_Tests);
  Serial.begin(115200);
}


void loop(){
  cli.Run();
  TracesDisplay();
}  


void Reset_Tests(void)
{
    byte return_val;
    Serial.println(F("\n########## Reset Tests ##########"));
    Serial.println(F("Requesting Reset..."));    
    KnxTpUart tpuart(Serial1, 0x1234, NORMAL);
    tpuart.SetDebugString(&traces);
    
    return_val = tpuart.Reset();
    TracesDisplay();
    Serial.print(F("Reset return val = ")); Serial.println(return_val);
}


void Attach_Tests()
// WARNING : IsAddressAssigned() function shall be made public (private by default) for this test
{
byte return_val;
  Serial.println(F("\n########## Attach tests  ##########"));

  {
    Serial.println(F("### Attach with error KNX_TPUART_ERROR_NOT_INIT_STATE (254)"));
    KnxComObject list[] = { KnxComObject(0x0000, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) , };
    KnxTpUart tpuart(Serial1, 0x1234, NORMAL);
    return_val = tpuart.AttachComObjectsList(list,sizeof(list)/sizeof(KnxComObject));
    Serial.print(F("Attach return val = ")); Serial.println(return_val);   
  }
  TracesDisplay();
  { 
    Serial.println(F("\n### Attach an ordered list with com attributes objects and no duplicate addr"));  
    KnxComObject list[] =
    {
    /* index 0 */ KnxComObject(0x0000, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 1 */ KnxComObject(0x0001, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 2 */ KnxComObject(0x0002, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 3 */ KnxComObject(0x0003, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 4 */ KnxComObject(0x0004, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 5 */ KnxComObject(0x0005, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 6 */ KnxComObject(0xFFFF, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    };
    KnxTpUart tpuart(Serial1, 0x1234, NORMAL);
    tpuart.SetDebugString(&traces);
    tpuart.Reset();
    return_val = tpuart.AttachComObjectsList(list,sizeof(list)/sizeof(KnxComObject));
    TracesDisplay();
    Serial.print(F("Attach return val = ")); Serial.println(return_val);
    for (byte i=0; i<sizeof(list)/sizeof(KnxComObject); i++)
    {
      byte index;
      return_val = tpuart.IsAddressAssigned(list[i].GetAddr(), index);
      Serial.print(F("Adress=")); Serial.print(list[i].GetAddr(), HEX);
      if (return_val)
      {
        Serial.print(F(" => index=")); Serial.println(index);           
      }
      else
      {
        Serial.println(F(" => Adress not found!"));
      }
    }
  }
  TracesDisplay();
  { 
    Serial.println(F("\n### Attach an unordered list with com attributes objects and no duplicate addr"));  
    KnxComObject list[] =
    {
    /* index 0 */ KnxComObject(0xFFFF, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 1 */ KnxComObject(0x0005, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 2 */ KnxComObject(0x0004, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 3 */ KnxComObject(0x0003, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 4 */ KnxComObject(0x0002, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 5 */ KnxComObject(0x0001, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 6 */ KnxComObject(0x0000, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    };
    KnxTpUart tpuart(Serial1, 0x1234, NORMAL);
    tpuart.SetDebugString(&traces);
    tpuart.Reset();
    tpuart.SetDebugString(&traces);
    return_val = tpuart.AttachComObjectsList(list,sizeof(list)/sizeof(KnxComObject));
    TracesDisplay();
    Serial.print(F("Attach return val = ")); Serial.println(return_val);
    for (byte i=0; i<sizeof(list)/sizeof(KnxComObject); i++)
    {
      byte index;
      return_val = tpuart.IsAddressAssigned(list[i].GetAddr(), index);
      Serial.print(F("Adress=")); Serial.print(list[i].GetAddr(), HEX);
      if (return_val)
      {
        Serial.print(F(" => index=")); Serial.println(index);           
      }
      else
      {
        Serial.println(F(" => Adress not found!"));
      }
    }
  }
  TracesDisplay();
  { 
    Serial.println(F("\n### Attach an unordered list with com attributes objects and duplicate addr"));  
    KnxComObject list[] =
    {
    /* index 0 */ KnxComObject(0xFFFF, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 1 */ KnxComObject(0xFFFF, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) , // duplicate @!!
    /* index 2 */ KnxComObject(0x0005, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 3 */ KnxComObject(0x0004, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 4 */ KnxComObject(0x0003, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 5 */ KnxComObject(0x0002, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 6 */ KnxComObject(0x0003, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) , // duplicate @!!
    /* index 7 */ KnxComObject(0x0001, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 8 */ KnxComObject(0x0003, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) , // duplicate @!!
    /* index 9 */ KnxComObject(0x0000, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 10*/ KnxComObject(0x0000, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) , // duplicate @!!
    };
    KnxTpUart tpuart(Serial1, 0x1234, NORMAL);
    tpuart.SetDebugString(&traces);
    tpuart.Reset();
    tpuart.SetDebugString(&traces);
    return_val = tpuart.AttachComObjectsList(list,sizeof(list)/sizeof(KnxComObject));
    TracesDisplay();
    Serial.print(F("Attach return val = ")); Serial.println(return_val);
    for (byte i=0; i<sizeof(list)/sizeof(KnxComObject); i++)
    {
      byte index;
      return_val = tpuart.IsAddressAssigned(list[i].GetAddr(), index);
      Serial.print(F("Adress=")); Serial.print(list[i].GetAddr(), HEX);
      if (return_val)
      {
        Serial.print(F(" => index=")); Serial.println(index);           
      }
      else
      {
        Serial.println(F(" => Adress not found!"));
      }
    }
  }
  TracesDisplay();
  { 
    Serial.println(F("\n### Attach an unordered list with non com attributes objects and duplicate addr"));  
    KnxComObject list[] =
    {
    /* index 0 */ KnxComObject(0xFFFF, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 1 */ KnxComObject(0xFFFF, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) , // duplicate @!!
    /* index 2 */ KnxComObject(0x0005, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , 0)                , // no com attribute !
    /* index 3 */ KnxComObject(0x0004, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 4 */ KnxComObject(0x0003, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 5 */ KnxComObject(0x0002, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 6 */ KnxComObject(0x0003, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) , // duplicate @!!
    /* index 7 */ KnxComObject(0x0001, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 8 */ KnxComObject(0x0003, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , 0               ) , // no com attrinute !
    /* index 9 */ KnxComObject(0x0000, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) ,
    /* index 10*/ KnxComObject(0x0000, KNX_DPT_1_001 /* 1.001 B1 DPT_Switch */ , COM_OBJ_LOGIC_IN) , // duplicate @!!
    };
    KnxTpUart tpuart(Serial1, 0x1234, NORMAL);
    tpuart.SetDebugString(&traces);
    tpuart.Reset();
    tpuart.SetDebugString(&traces);
    return_val = tpuart.AttachComObjectsList(list,sizeof(list)/sizeof(KnxComObject));
    TracesDisplay();
    Serial.print(F("Attach return val = ")); Serial.println(return_val);
    for (byte i=0; i<sizeof(list)/sizeof(KnxComObject); i++)
    {
      byte index;
      return_val = tpuart.IsAddressAssigned(list[i].GetAddr(), index);
      Serial.print(F("Adress=")); Serial.print(list[i].GetAddr(), HEX);
      if (return_val)
      {
        Serial.print(F(" => index=")); Serial.println(index);           
      }
      else
      {
        Serial.println(F(" => Adress not found!"));
      }
    }
  }
  TracesDisplay();
}


void Init_Tests(void)
{
  byte return_val;
  Serial.println(F("\n########## Init Tests ##########"));
    
  {
    KnxTpUart tpuart(Serial1, 0x1234, NORMAL);
    tpuart.SetDebugString(&traces);

    Serial.println(F("### Testing NORMAL mode Init with error KNX_TPUART_ERROR_NOT_INIT_STATE (254)"));
    return_val = tpuart.Init();
    TracesDisplay();
    Serial.print(F("Init return val = ")); Serial.println(return_val);

    Serial.println(F("### Testing NORMAL mode Init with error but empty list warning"));
    Serial.println(F("Requesting Reset..."));
    tpuart.Reset();
    return_val = tpuart.Init();
    TracesDisplay();
    Serial.print(F("Init return val = ")); Serial.println(return_val);

    Serial.println(F("### Testing NORMAL mode Init with error KNX_TPUART_ERROR_NULL_EVT_CALLBACK_FCT (253)"));
    tpuart.AttachComObjectsList(objList,sizeof(objList)/sizeof(KnxComObject));
    return_val = tpuart.Init();
    TracesDisplay();
    Serial.print(F("Init return val = ")); Serial.println(return_val);

    Serial.println(F("### Testing NORMAL mode Init with error KNX_TPUART_ERROR_NULL_ACK_CALLBACK_FCT (252)"));
    tpuart.SetEvtCallback(eventCallback);
    return_val = tpuart.Init();
    TracesDisplay();
    Serial.print(F("Init return val = ")); Serial.println(return_val);

    Serial.println(F("### Testing NORMAL mode Init with no error"));
    tpuart.SetAckCallback(ackCallback);
    return_val = tpuart.Init();
    TracesDisplay();
    Serial.print(F("Init return val = ")); Serial.println(return_val);

    Serial.println(F("### Testing NORMAL mode Init after hot reset"));
    Serial.println(F("Requesting Reset..."));
    tpuart.Reset();
    return_val = tpuart.Init();
    TracesDisplay();
    Serial.print(F("Init return val = ")); Serial.println(return_val);
  }
    Serial.print(traces); traces=""; // will show destructor traces      
  {
    Serial.println(F("### Testing MONITOR mode"));
    KnxTpUart tpuart(Serial1, 0x1234, BUS_MONITOR);
    tpuart.SetDebugString(&traces);
    Serial.println(F("Requesting Reset..."));
    tpuart.Reset();
    return_val = tpuart.Init();
    TracesDisplay();
    Serial.print(F("Init return val = ")); Serial.println(return_val);
  }
}


void Bus_Monitoring(void)
{
  Serial.println(F("\n########## Bus Monitoring  ##########"));
  Serial.println(F("Press Enter to stop  the test..."));
  KnxTpUart tpuart(Serial1, 0x1234, BUS_MONITOR);
  tpuart.SetDebugString(&traces);
  Serial.println(F("Requesting Reset..."));
  tpuart.Reset();
  tpuart.Init();
  TracesDisplay();  
  byte running = 1;
  while(running)
  {
    type_MonitorData data;
    if(Pulse400us())
    {
      if (tpuart.GetMonitoringData(data))
      {
        if (data.isEOP) Serial.println("<eop>");
        else 
        { 
          Serial.print(data.dataByte,HEX);
          Serial.print(" ");
        }
      }
    }
    if (Serial.available()) running = 0;
  }
  while(Serial.available()) Serial.read(); // flush Serial Rx buffer
}


void Normal_Rx(void)
{
  Serial.println(F("\n########## RX Tests  ##########"));
  Serial.println(F("Press Enter to stop  the test..."));
  KnxTpUart tpuart(Serial1, 0x1234, NORMAL);
  KnxTelegram &tg = tpuart.GetReceivedTelegram();
  tpuart.SetDebugString(&traces);
  tpuart.Reset();
  tpuart.SetEvtCallback(eventCallback);
  tpuart.SetAckCallback(ackCallback);
  tpuart.AttachComObjectsList(objList,sizeof(objList)/sizeof(KnxComObject));
  tpuart.Init();
  TracesDisplay();
  newTgEvt = 0;
  Serial.println(F("Waiting for addressed telegrams..."));
  byte running = 1;
  while(running)
  {
    if(Pulse400us()) tpuart.RXTask();
    if (newTgEvt)
    {
      newTgEvt--;
      Serial.print("Telegram received, index="); Serial.println(tpuart.GetTargetedComObjectIndex());
      PrintTelegramInfo(tg);
    }
    if (Serial.available()) running = 0;
  }
  while(Serial.available()) Serial.read(); // flush Serial Rx buffer
}


void Normal_Rx_ResetEvt(void)
{
  Serial.println(F("\n########## Reset Event reception test  ##########"));
  KnxTpUart tpuart(Serial1, 0x1234, NORMAL);
  KnxTelegram &tg = tpuart.GetReceivedTelegram();
  tpuart.SetDebugString(&traces);
  tpuart.Reset();
  tpuart.SetEvtCallback(eventCallback);
  tpuart.SetAckCallback(ackCallback);
  tpuart.AttachComObjectsList(objList,sizeof(objList)/sizeof(KnxComObject));
  tpuart.Init();
  TracesDisplay();
  resetEvt = 0;
  Serial.print(F("Sending Reset Event..."));
  tpuart.DEBUG_SendResetCommand();
  byte running = 1;
  while(running)
  {
    if(Pulse400us()) tpuart.RXTask();
    if (resetEvt)
    {
      Serial.println(F("Reset Event received!"));      
      running = 0;
    }
  }  
}


void Normal_Rx_StateEvt(void)
{
  Serial.println(F("\n########## State Event reception test  ##########"));  
  KnxTpUart tpuart(Serial1, 0x1234, NORMAL);
  KnxTelegram &tg = tpuart.GetReceivedTelegram();
  tpuart.SetDebugString(&traces);
  tpuart.Reset();
  tpuart.SetEvtCallback(eventCallback);
  tpuart.SetAckCallback(ackCallback);
  tpuart.AttachComObjectsList(objList,sizeof(objList)/sizeof(KnxComObject));
  tpuart.Init();
  TracesDisplay();
  newStateEvt = 0;
  Serial.print(F("Sending State Event..."));
  tpuart.DEBUG_SendStateReqCommand();
  byte running = 1;
  while(running)
  {
    if(Pulse400us()) tpuart.RXTask();
    if (newStateEvt)
    {
      Serial.println(F("State Event received!"));      
      running = 0;
    }
  }  
}


void Normal_Tx_Val1(void)
{
  Serial.println(F("\n########## Telegram Transmission Test (value 1)  ##########"));  
  KnxTpUart tpuart(Serial1, 0x1234, NORMAL);
  tpuart.SetDebugString(&traces);
  tpuart.Reset();
  tpuart.SetEvtCallback(eventCallback);
  tpuart.SetAckCallback(ackCallback);
  tpuart.AttachComObjectsList(objList,sizeof(objList)/sizeof(KnxComObject));
  tpuart.Init();
  TracesDisplay();
  ackEvt = 0;
  Serial.print(F("Sending telegram..."));
  
  KnxTelegram tg;
  tg.SetTargetAddress(0x0001);
  tg.SetCommand(KNX_COMMAND_VALUE_WRITE);
  tg.SetFirstPayloadByte(1);
  tg.UpdateChecksum();
  tpuart.SendTelegram(tg);
  byte running = 1;
  while(running)
  {
    if(Pulse400us()) tpuart.RXTask();
    if(Pulse800us()) tpuart.TXTask();
    if ((ackEvt) && (ackVal==ACK_RESPONSE))
    {
      running = 0;
      Serial.println(F("ACK received!"));
    }
  }  
  Serial.println(F("Press Enter to continue..."));
  running = 1;
  while(running) if (Serial.available()) running = 0;
  while(Serial.available()) Serial.read(); // flush Serial Rx buffer
}


void Normal_Tx_Val0(void)
{
  Serial.println(F("\n########## Telegram Transmission Test (value 0)  ##########"));  
  KnxTpUart tpuart(Serial1, 0x1234, NORMAL);
  tpuart.SetDebugString(&traces);
  tpuart.Reset();
  tpuart.SetEvtCallback(eventCallback);
  tpuart.SetAckCallback(ackCallback);
  tpuart.AttachComObjectsList(objList,sizeof(objList)/sizeof(KnxComObject));
  tpuart.Init();
  TracesDisplay();
  ackEvt = 0;
  Serial.print(F("Sending telegram..."));
  
  KnxTelegram tg;
  tg.SetTargetAddress(0x0001);
  tg.SetCommand(KNX_COMMAND_VALUE_WRITE);
  tg.SetFirstPayloadByte(0);
  tg.UpdateChecksum();
  tpuart.SendTelegram(tg);
  byte running = 1;
  while(running)
  {
    if(Pulse400us()) tpuart.RXTask();
    if(Pulse800us()) tpuart.TXTask();
    if ((ackEvt) && (ackVal==ACK_RESPONSE)) 
    {
      running = 0;
      Serial.println(F("ACK received!"));
    }
  }
  Serial.println(F("Press Enter to continue..."));
  running = 1;
  while(running) if (Serial.available()) running = 0;
  while(Serial.available()) Serial.read(); // flush Serial Rx buffer  
}


void Normal_Tx_NoAck(void)
{
  Serial.println(F("\n########## Telegram Transmission with No Ack ##########"));  
  KnxTpUart tpuart(Serial1, 0x1234, NORMAL);
  tpuart.SetDebugString(&traces);
  tpuart.Reset();
  tpuart.SetEvtCallback(eventCallback);
  tpuart.SetAckCallback(ackCallback);
  tpuart.AttachComObjectsList(objList,sizeof(objList)/sizeof(KnxComObject));
  tpuart.Init();
  TracesDisplay();
  ackEvt = 0;
  Serial.print(F("Sending telegram..."));
  
  KnxTelegram tg;
  tg.SetTargetAddress(0xAAAA);
  tg.SetCommand(KNX_COMMAND_VALUE_WRITE);
  tg.SetFirstPayloadByte(0);
  tg.UpdateChecksum();
  tpuart.SendTelegram(tg);
  byte running = 1;
  while(running)
  {
    if(Pulse400us()) tpuart.RXTask();
    if(Pulse800us()) tpuart.TXTask();
    if ((ackEvt)  && (ackVal==NACK_RESPONSE)) 
    {
      running = 0;
      Serial.println(F("No Ack received!"));
    }
  }
}

void Normal_Tx_Timeout(void)
{
  Serial.println(F("\n########## Test Telegram Transmission with No answer timeout ##########"));  
  KnxTpUart tpuart(Serial1, 0x1234, NORMAL);
  tpuart.SetDebugString(&traces);
  tpuart.Reset();
  tpuart.SetEvtCallback(eventCallback);
  tpuart.SetAckCallback(ackCallback);
  tpuart.AttachComObjectsList(objList,sizeof(objList)/sizeof(KnxComObject));
  tpuart.Init();
  TracesDisplay();
  ackEvt = 0;
  Serial.print(F("Sending telegram..."));
  
  KnxTelegram tg;
  tg.SetTargetAddress(0xAAAA);
  tg.SetCommand(KNX_COMMAND_VALUE_WRITE);
  tg.SetFirstPayloadByte(0);
  tg.UpdateChecksum();
  tpuart.SendTelegram(tg);
  byte running = 1;
  while(running)
  {
    // the RX task is not run so that no answer is received from the TPUART
    // tpuart.RXTask();
    if(Pulse800us()) tpuart.TXTask();
    if ((ackEvt)  && (ackVal==NO_ANSWER_TIMEOUT)) 
    {
      running = 0;
      Serial.println(F("No answer timeout received!"));
    }
  }
}


void Normal_Tx_Reset(void)
{
  Serial.println(F("\n########## Test Telegram Transmission with Reset response ##########"));  
  KnxTpUart tpuart(Serial1, 0x1234, NORMAL);
  tpuart.SetDebugString(&traces);
  tpuart.Reset();
  tpuart.SetEvtCallback(eventCallback);
  tpuart.SetAckCallback(ackCallback);
  tpuart.AttachComObjectsList(objList,sizeof(objList)/sizeof(KnxComObject));
  tpuart.Init();
  TracesDisplay();
  ackEvt = 0;
  Serial.print(F("Sending telegram..."));
  
  KnxTelegram tg;
  tg.SetTargetAddress(0xAAAA);
  tg.SetCommand(KNX_COMMAND_VALUE_WRITE);
  tg.SetFirstPayloadByte(0);
  tg.UpdateChecksum();
  tpuart.SendTelegram(tg);
  byte running = 1;
  byte counter = 0;
  ackEvt = 0;
  resetEvt = 0;
  while(running)
  {
    if(Pulse400us()) tpuart.RXTask();
    if(Pulse800us()) { tpuart.TXTask(); counter ++; }
    // the telegram is 9 bytes long, so it will be sent in 9 TXTask() executions
    // we send a RESET request right after the telegram transmission is done
    if(counter == 9) tpuart.DEBUG_SendResetCommand(); 

    if ( (ackEvt) && (ackVal==TPUART_RESET_RESPONSE)) 
    {
      running = 0;
      Serial.println(F("ack tpuart reset received !"));
    }
    if (resetEvt)
    {
      Serial.println(F("Reset Event received!"));      
    }
    counter++;
  }
}


void All_Tests(void)
{
  Reset_Tests(); TracesDisplay();
  Attach_Tests(); TracesDisplay();
  Init_Tests(); TracesDisplay();
  Bus_Monitoring(); TracesDisplay();
  Normal_Rx(); TracesDisplay();
  Normal_Rx_ResetEvt(); TracesDisplay();
  Normal_Rx_StateEvt(); TracesDisplay();
  Normal_Tx_Val0(); TracesDisplay();
  Normal_Tx_Val1(); TracesDisplay();
  Normal_Tx_NoAck(); TracesDisplay();
  Normal_Tx_Timeout(); TracesDisplay();
  Normal_Tx_Reset(); TracesDisplay();
}
