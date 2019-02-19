#include <avr/io.h> 
#include <avr/interrupt.h> 
 
#define PIN_ICP1 8
 
volatile unsigned int T1capture, lastT1capture, period ; 
volatile unsigned int ReceiverBuffer[50];
volatile byte ReceiverBufferIndex = 0;
 
void setup() 
{ 
  Serial.begin(9600); 
  //pinMode(8, INPUT_PULLUP); // PB0 = ICP1 = arduino pin 8
  Init(); 
  Serial.println("Init finished"); 
  for(int i = 0; i<50; i++) {
    ReceiverBuffer[i] = 0;
  }
} 
 
void loop() {
  while(1) {
    if (ReceiverBufferIndex == 50) {
      for(int i = 0; i<50; i++) {
        Serial.println(ReceiverBuffer[i]);
      }
      ReceiverBufferIndex = 51;
    }
  }
} 
 
void Init() { 
  // init timer
  // set capture to positive edge, full counting rate
  TCCR1B = (1<<ICES1) + 1; 
  // turn on timer1 interrupt-on-capture
  TIMSK1 = (1<<ICIE1) ;
  // turn off other timer1 functions
  TCCR1A = 0;
  sei(); //ENABLING GLOBAL INTERRUPTS 
  // set up the input capture pin, ICP1, which corresponds to arduino D8
  pinMode(PIN_ICP1, INPUT);
  digitalWrite(PIN_ICP1,HIGH);
} 
 
// ISR for falling edge
ISR(TIMER1_CAPT_vect) { 
    // read timer1 input capture register
    T1capture = ICR1 ; 
    // compute time between captures
    period =  T1capture - lastT1capture;
    lastT1capture = T1capture ;
    if (ReceiverBufferIndex < 50) {
      ReceiverBuffer[ReceiverBufferIndex] = period;
      ReceiverBufferIndex++;
    }
}
