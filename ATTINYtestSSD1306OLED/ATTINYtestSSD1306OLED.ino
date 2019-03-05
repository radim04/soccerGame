#include <Tiny4kOLED.h>
void setup() {
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

void loop() {
  updateDisplay();
  delay(50);
}

void updateDisplay() {
  // Clear the half of memory not currently being displayed.
  oled.clear();
  // Position the text cursor
  // In order to keep the library size small, text can only be positioned
  // with the top of the font aligned with one of the four 8 bit high RAM pages.
  // The Y value therefore can only have the value 0, 1, 2, or 3.
  // usage: oled.setCursor(X IN PIXELS, Y IN ROWS OF 8 PIXELS STARTING WITH 0);
  oled.setCursor(0, 1);
  // Write text to oled RAM (which is not currently being displayed).
  oled.print(F("ms: "));
  // Write the number of milliseconds since power on.
  oled.print(millis());
  // Swap which half of RAM is being written to, and which half is being displayed.
  // This is equivalent to calling both switchRenderFrame and switchDisplayFrame.
  oled.switchFrame();
}
