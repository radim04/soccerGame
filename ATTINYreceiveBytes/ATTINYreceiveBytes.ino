#include <Tiny4kOLED.h>
#include <avr/io.h> 
#include <avr/interrupt.h> 
 
#define PIN_ICP1 8
#define TIMER_COUNTER_RESET 4000
#define TIMER_COUNTER_0 800
#define TIMER_COUNTER_1 1200
#define TIMER_COUNTER_LIMIT_1 1000
#define TIMER_COUNTER_LIMIT_RESET 2000
#define RECEIVER_BUFFER_LENGTH 3
 
volatile unsigned int T1capture, lastT1capture, period ; 
volatile byte receiverBuffer[50];
volatile byte receiverBufferByteIndex = 0;
volatile byte receiverBufferBitIndex = B10000000;
volatile boolean receiverBufferIsReady = false;
 
void setup() 
{ 
  setupOLED();
  Init(); 
  printOLED(0, "Init finished"); 
  for(int i = 0; i<50; i++) {
    receiverBuffer[i] = 0;
  }
  receiverBufferIsReady = false;
} 

void setupOLED() {
  // Send the initialization sequence to the oled. This leaves the display turned off
  oled.begin();
  oled.setFont(FONT8X16);
  // Clear the memory before turning on the display
  oled.clear();
  // Turn on the display
  oled.on();
  // Switch the half of RAM that we are writing to, to be the half that is non currently displayed
  oled.switchRenderFrame();
}

void printOLED(uint8_t y, char *message) {
  // Clear the half of memory not currently being displayed.
  oled.clear();
  // Position the text cursor
  // In order to keep the library size small, text can only be positioned
  // with the top of the font aligned with one of the four 8 bit high RAM pages.
  // The Y value therefore can only have the value 0, 1, 2, or 3.
  // usage: oled.setCursor(X IN PIXELS, Y IN ROWS OF 8 PIXELS STARTING WITH 0);
  oled.setCursor(0, y);
  // Write text to oled RAM (which is not currently being displayed).
  oled.print(message);
  // Swap which half of RAM is being written to, and which half is being displayed.
  // This is equivalent to calling both switchRenderFrame and switchDisplayFrame.
  oled.switchFrame();  
}

void loop() {
  while(1) {
    if (receiverBufferIsReady) {
      oled.clear();
      oled.setCursor(0, 0);
      for(int i = 0; i<RECEIVER_BUFFER_LENGTH; i++) {
        oled.print(receiverBuffer[i]);
        oled.print(F("|"));
      }
      oled.switchFrame();
//      Serial.println(CRC8(receiverBuffer, RECEIVER_BUFFER_LENGTH));
      while (true) {
        
      }
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

// ISR for falling edge
ISR(TIMER1_CAPT_vect) { 
  // read timer1 input capture register
  T1capture = ICR1 ; 
  // compute time between captures
  period =  T1capture - lastT1capture;
  lastT1capture = T1capture;
  if (receiverBufferIsReady == false && period > 0) {
    // set current bit
    if (period < TIMER_COUNTER_LIMIT_1) {
      receiverBuffer[receiverBufferByteIndex] &= ~receiverBufferBitIndex; // set bit to LOW
    } else if (period < TIMER_COUNTER_LIMIT_RESET) {
      receiverBuffer[receiverBufferByteIndex] |= receiverBufferBitIndex; // set bit to HIGH
    } else {
      // reset
      receiverBufferByteIndex = 0;
      receiverBufferBitIndex = B10000000;
    }
    if (period < TIMER_COUNTER_LIMIT_RESET) {
      // move to the next bit in the buffer
      receiverBufferBitIndex = receiverBufferBitIndex >> 1;
      if (!receiverBufferBitIndex) {
        // bit index overflow -> move to the leftmost bit of the next byte
        receiverBufferBitIndex = B10000000;
        receiverBufferByteIndex++;
        if (receiverBufferByteIndex >= RECEIVER_BUFFER_LENGTH) {
          receiverBufferIsReady = true; // finished!
          receiverBufferByteIndex = 0;
          receiverBufferBitIndex = B10000000;
        }
      } 
    }  
  }
}
