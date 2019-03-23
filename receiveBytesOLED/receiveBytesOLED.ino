#include <avr/io.h> 
#include <avr/interrupt.h> 
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
 
#define PIN_ANALOG 3
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
volatile int timer=0;
volatile bool state=0;
 
void setup() { 
  pinMode(PIN_ANALOG, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  InitTimer0(); 
  InitTimer1(); 
  sei(); //enable interrupts
  InitOLED();
  for(int i = 0; i<50; i++) {
    receiverBuffer[i] = 0;
  }
  receiverBufferIsReady = false;
} 
 
void loop() {
  if (receiverBufferIsReady) {
    display.clearDisplay();
    display.setCursor(0, 0);
    for(int i = 0; i<RECEIVER_BUFFER_LENGTH; i++) {
      display.print(F("|"));
      display.print(receiverBuffer[i]);
    }
    display.println(F("|"));
    display.println(CRC8(receiverBuffer, RECEIVER_BUFFER_LENGTH));
    display.display();
    analogWrite(PIN_ANALOG, receiverBuffer[0]);
    receiverBufferIsReady = false;
  }
  if (timer>=10000) {
    state=!state;
    timer=0;
    digitalWrite(13,state);
  } 
} 

void InitTimer0() { 
  // interrupt frequency 10kHz (http://www.8bit-era.cz/arduino-timer-interrupts-calculator.html)
  TCCR0A = 0; // set entire TCCR0A register to 0
  TCCR0B = 0; // same for TCCR0B
  TCNT0  = 0; // initialize counter value to 0
  // set compare match register for 10000 Hz increments
  OCR0A = 199; // = 16000000 / (8 * 10000) - 1 (must be <256)
  // turn on CTC mode
  TCCR0B |= (1 << WGM01);
  // Set CS02, CS01 and CS00 bits for 8 prescaler
  TCCR0B |= (0 << CS02) | (1 << CS01) | (0 << CS00);
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);
}
 
void InitTimer1() { 
  // used for communication with sender
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

void InitOLED() { 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.println(F("Init finished"));
  display.display();
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

ISR(TIMER0_COMPA_vect) {   
  //timer++;
}

// ISR for falling edge (handle communication with sender)
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
