/* 
Send bytes on a one-wire-like bus
*/

#define transmitterPin 13
#define NOP __asm__ __volatile__ ("nop\n\t")
#define TIMER_COUNTER_RESET 61536  //65536-4000
#define TIMER_COUNTER_0 64736  //65536-800
#define TIMER_COUNTER_1 64336  //65536-1200
#define TRANSMIT_BUFFER_LENGTH 4

int timer1_counter;

volatile byte transmitBuffer[] = {15, 15, 15, 0};
volatile byte transmitBufferByteIndex = 0;
volatile byte transmitBufferBitIndex = 0;
volatile boolean transmitBufferIsTransmitting = false;

void setup()
{
  pinMode(transmitterPin, OUTPUT);

  // compute CRC for transmitted data

  // initialize timer1 
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;

  // Set timer1_counter to the correct value for our interrupt interval
  //timer1_counter = 64911;   // preload timer 65536-16MHz/256/100Hz
  //timer1_counter = 64286;   // preload timer 65536-16MHz/256/50Hz
  timer1_counter = 65536-800;   // trigger every 50us
  
  TCNT1 = TIMER_COUNTER_RESET;   // preload timer
  TCCR1B |= (1 << CS10);    // no prescaling 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
  transmitBufferIsTransmitting = true;
}

ISR(TIMER1_OVF_vect)        // TIMER1 interrupt service routine 
{
  if( transmitBufferIsTransmitting ) {
    // first preload timer (TCNT1)
    if( !transmitBufferBitIndex ) {
      // send reset puls (a long puls)
      TCNT1 = TIMER_COUNTER_RESET;
      transmitBufferBitIndex = B10000000;
    } else {
      // get current bit from transmitting buffer
      byte currentBit = transmitBuffer[transmitBufferByteIndex] & transmitBufferBitIndex;
      if( currentBit ) {
        TCNT1 = TIMER_COUNTER_1; // preload timer with long interval (HIGH)
      } else {
        TCNT1 = TIMER_COUNTER_0; // preload timer with short interval (LOW)
      }
      // shift index by one bit to the right
      transmitBufferBitIndex = transmitBufferBitIndex >> 1;
      if( !transmitBufferBitIndex ) {
        // bit index overflow -> move to the leftmost bit of the next byte
        transmitBufferBitIndex = B10000000;
        transmitBufferByteIndex++;
        if( transmitBufferByteIndex >= TRANSMIT_BUFFER_LENGTH ) {
          transmitBufferIsTransmitting = false; // finished!
          transmitBufferByteIndex = 0;
          transmitBufferBitIndex = 0;
        }
      }
    }
    // send short pulse
    digitalWrite(transmitterPin, HIGH);
    NOP;
    NOP;
    NOP;
    NOP;
    NOP;
    digitalWrite(transmitterPin, LOW);
  }
}

//CRC-8 - based on the CRC8 formulas by Dallas/Maxim
//code released under the therms of the GNU GPL 3.0 license
byte CRC8(const byte *data, byte len) {
  byte crc = 0x00;
  while (len--) {
    byte extract = *data++;
    for (byte tempI = 8; tempI; tempI--) {
      byte sum = (crc ^ extract) & 0x01;
      crc >>= 1;
      if (sum) {
        crc ^= 0x8C;
      }
      extract >>= 1;
    }
  }
  return crc;
}

void loop()
{
  if (!transmitBufferIsTransmitting) {
    transmitBuffer[0] = map(analogRead(A0), 0, 1023, 0, 255);
    transmitBufferIsTransmitting = true;
  }
}

