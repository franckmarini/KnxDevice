#include <KnxDevice.h> // /!\ Turn "ACTIONRINGBUFFER_STAT" define on in ActionRingBuffer.h to allow statistics info
#include <Cli.h> // command line interpreter lib available at https://github.com/franckmarini/Cli

Cli cli = Cli(Serial);

RingBuffer<long, 8> buffer; // Ring buffer containing up to 8 long values
long counter = 1;

void Add(void);
void Pop(void);
void Info(void);
void Scenario(void);


void setup() {
  cli.RegisterCmd("a",&Add);
  cli.RegisterCmd("p",&Pop);
  cli.RegisterCmd("i",&Info);
  cli.RegisterCmd("s",&Scenario);
  Serial.begin(115200);
}


void loop() {
  cli.Run();
}  


void Info() {
  String str;
  str = " => Info() :\n"; buffer.Info(str);
  Serial.print(str);
}


void Add(void) {
  buffer.Append(counter);
  Serial.print(F("Value ")); Serial.print(counter,DEC); Serial.println(F(" appended"));
  counter++; 
}


void Pop(void) {
  boolean result;
  long popVal;
  result = buffer.Pop(popVal);
  if (result) {
  Serial.print(F("Popped value ")); Serial.println(popVal,DEC);
  }
  else Serial.println(F("No value popped : buffer empty!"));
}


void Scenario(void) {
  Info(); // Buffer empty
  for( int i=1; i<=7; i++) Add(); // Append 7 elements (value 1 to 7)
  Info(); // show 7 elements
  Add(); // Append value 7
  Info(); // show 8 elements
  Pop(); //  value 1 popped
  Info(); // show 7 elements
  for( int i=1; i<=7; i++) Pop(); // Pop 7 elements (value 2 to 8)
  Info(); // buffer is empty
  Pop(); // tell buffer is empty
  Serial.println("counter value reset"); counter = 1;
  for( int i=1; i<=8; i++) Add(); // Append 8 elements (value 1 to 8)
  Info(); // show 8 elements
  Add(); //  append value 9, which overwrites value 1
  Info(); // tell one element is lost
  Add(); //  append value 10, which overwrites value 2
  Info(); // tell 2 elements are lost
  for( int i=1; i<=8; i++) Pop(); // Pop value 3 to 10
  Pop(); // tell buffer is empty
}


