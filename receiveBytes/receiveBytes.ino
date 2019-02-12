#include <avr/io.h> 
#include <avr/interrupt.h> 
 
// Set up input capture mode
// Set up timers
// Set up port for output
 
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
  //TCNT1=0;                         //SETTING INTIAL TIMER VALUE
  //TCCR1B|=(1<<ICES1);              //SETTING FIRST CAPTURE ON RISING EDGE ,(TCCR1B = TCCR1B | (1<<ICES1)
  //TIMSK1|=(1<<ICIE1);              //ENABLING INPUT CAPTURE INTERRUPT
  // start timer
  //TCCR1B|=(1<<CS10);              //STARTING TIMER WITH NO PRESCALING
//set up timer1 for full speed and
//capture an edge on analog comparator pin B.3 
// Set capture to positive edge, full counting rate
TCCR1B = (1<<ICES1) + 1; 
// Turn on timer1 interrupt-on-capture
TIMSK1 = (1<<ICIE1) ;
// turn off other timer1 functions
TCCR1A = 0;
  sei();                          //ENABLING GLOBAL INTERRUPTS 
  // Set up the Input Capture pin, ICP1, which corresponds to Arduino D8
  pinMode(8, INPUT);
  digitalWrite(8,HIGH);
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
