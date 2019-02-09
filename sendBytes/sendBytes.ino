/* 
Example Timer1 Interrupt
Flash LED every second
*/

#define ledPin 13
#define NOP __asm__ __volatile__ ("nop\n\t")
#define TIMER_COUNTER_0 64736  //65536-800
#define TIMER_COUNTER_1 64536  //65536-1000
#define TRANSMIT_BUFFER_LENGTH 2

int timer1_counter;

volatile byte transmitBuffer[] = {123, 255};
volatile byte transmitBufferByteIndex = 0;
volatile byte transmitBufferBitIndex = B10000000;
volatile boolean transmitBufferIsTransmitting = false;

void setup()
{
  pinMode(ledPin, OUTPUT);

  // initialize timer1 
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;

  // Set timer1_counter to the correct value for our interrupt interval
  //timer1_counter = 64911;   // preload timer 65536-16MHz/256/100Hz
  //timer1_counter = 64286;   // preload timer 65536-16MHz/256/50Hz
  timer1_counter = 65536-800;   // trigger every 50us
  
  TCNT1 = timer1_counter;   // preload timer
  //TCCR1B |= (1 << CS12);    // 256 prescaler 
  TCCR1B |= (1 << CS10);    // no prescaling 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
  transmitBufferIsTransmitting = true;
}

ISR(TIMER1_OVF_vect)        // interrupt service routine 
{
  if( transmitBufferIsTransmitting ) {
    // get current bit from transmitting buffer
    byte currentBit = transmitBuffer[transmitBufferByteIndex] & transmitBufferBitIndex;
    if( currentBit ) {
      TCNT1 = TIMER_COUNTER_1; // preload timer with long interval
    } else {
      TCNT1 = TIMER_COUNTER_0; // preload timer with short interval
    }
    // shift index by one bit to the right
    transmitBufferBitIndex = transmitBufferBitIndex >> 1;
    if( !transmitBufferBitIndex ) {
      // bit index overflow -> move to the leftmost bit of the next byte
      transmitBufferBitIndex = B10000000;
      transmitBufferByteIndex++;
      if( transmitBufferByteIndex >= TRANSMIT_BUFFER_LENGTH ) {
        transmitBufferIsTransmitting = false; // finished!
      }
    }
  }
  //digitalWrite(ledPin, digitalRead(ledPin) ^ 1);
  PORTB = B00100000; //Turn LED on.
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  PORTB = B00000000; //Turn LED off.
}

void loop()
{
  // your program here...
}

