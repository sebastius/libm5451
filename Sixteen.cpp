#include "Arduino.h"
#include "Sixteen.h"
#include <avr/pgmspace.h>

// constructor
Sixteen::Sixteen() {
  numScreens = 0;
}

// add your screens left-to-right
void Sixteen::addScreen(int dataPin, int clockPin) {

  if (numScreens == maxNumScreens) { return; }

  dataPins[numScreens]  = dataPin;
  clockPins[numScreens] = clockPin;

  // open the required pins
  pinMode(dataPin,  OUTPUT);
  pinMode(clockPin, OUTPUT);

  // ensure we're starting low
  digitalWrite(dataPin,  LOW);
  digitalWrite(clockPin, LOW);

  numScreens++;
}

// Sixteen::writeBit sends a single bit to the data stream, also
// pulsing the clock.
void Sixteen::writeBit(int screen, bool bit) {

  // set the bit
  digitalWrite(dataPins[screen], bit);

  // pulse the clock
  digitalWrite(clockPins[screen], HIGH);
  delayMicroseconds(10);
  digitalWrite(clockPins[screen], LOW);
}

// Sixteen::writeMask sets all 32 LEDs at once using a bitmask.
void Sixteen::writeMask(int screen, uint32_t frame) {

  // write the 'enable' bit and the first disconnected pin
  writeBit(screen, true); writeBit(screen, false);

  // write the bitmask, bit by bit
  for (int i = 0; i < 32; i++) {
    writeBit(screen, (bool) ((frame >> i) & 1));
  }

  // write to the other two disconnected pins
  writeBit(screen, false); writeBit(screen, false);
}

// make a 16-bit digit out of a character. Note that this is not
// right-side or left-side specific and would not be displayable on
// its own. Use 'print' for that.
uint16_t Sixteen::charMask(char c) {

  // lower-case letters are treated as capitals
  if (c >= 'a' && c <= 'z') { c -= 'z' - 'Z'; }

  // if you ask out of scope, the response is a blank digit
  if ((c < 0) || (c > lastChar)) { return 0u; }

  // otherwise, consult the font map
  return font[c];
}

void Sixteen::writeChars(int screen, char lchar, char rchar) {

  // left digit starts at bit 8 and continues t/m bit 23, so we'll push our
  // idealized character 8 bits forward to fill the left digit
  uint32_t lframe = (uint32_t) charMask(lchar);
  lframe <<= 8;

  // right digit takes up 0 t/m 7 and 24 t/m 31 (weird, right?) so we'll split
  // the bit in two, printing the bottom as normal and cutting off and pushing
  // forward the top half of the bottom one
  uint32_t rframe_high, rframe_low;
  rframe_low = (uint32_t) charMask(rchar);

  rframe_high = rframe_low & 0xff00u;
  rframe_low &= 0x00ffu;

  // push the high segment to its place
  rframe_high <<= 16;

  // write the segments
  writeMask(screen, rframe_low | lframe | rframe_high);
}


/* write a scrolling message to the screen sequence
 * remember: screens are added and numbered left-to-right
 *
 *         [][]   [][]   [][]   [][] ...
 * digit:  0  1   2  3   4  5   6  7 ...
 * screen:  #0     #1     #2     #3  ...
 */
void Sixteen::scroll(const char *message, int delay_ms) {

  // ignore empty strings
  if ('\0' == *message) { return; }

  // we actually iterate through the string with the digit on the far right
  // side of the chain of 16-segment displays
  const char *farRight = message;
  bool stringExhausted = false;

  // in the first part of the animation, the string starts to crawl from the
  // right to the left side of the digit-chain. every digit that hasn't yet
  // seen the first char of the message displays a blank char.
  int numDigits = (numScreens * 2);
  int lastDigit = numDigits - 1;
  int thisDigit;

  // we start lots of left-padding: every digit but the last
  char highestDigitPadded = lastDigit - 1;

  // we start with no right-padding yet: we'll add it when the string is over
  char lowestDigitPadded  = lastDigit + 1;

  // now let's print the string
  char left, right;
  while (lowestDigitPadded >= 0) {

    // at each animation frame, we have to print to each screen
    for (int screen = numScreens - 1; screen >= 0; screen--) {

      // determine the right-side character
      thisDigit = (screen * 2) + 1;
      if (highestDigitPadded >= thisDigit) {
        right = ' ';
      } else if (lowestDigitPadded <= thisDigit) {
        right = ' ';
      } else {
        right = *(farRight - (lastDigit - thisDigit) + (numDigits - lowestDigitPadded));
      }

      // determine the left-side character
      thisDigit = (screen * 2);
      if (highestDigitPadded >= thisDigit) {
        left = ' ';
      } else if (lowestDigitPadded <= thisDigit) {
        left = ' ';
      } else {
        left = *(farRight - (lastDigit - thisDigit) + (numDigits - lowestDigitPadded));
      }

      // write calculated characters to this screen
      writeChars(screen, left, right);
    }

    // take away a digit of left-padding if there's any left
    if (highestDigitPadded >= 0) { highestDigitPadded--; }

    // if the string is exhausted, add a digit of right-padding
    if (stringExhausted) {
      lowestDigitPadded--;

    // advance the string if there's any string left
    } else {
      farRight++;
      stringExhausted = ('\0' == *farRight);
    }

    // wait a bit so humans can see this frame
    delay(delay_ms);
  }
}

// display each digit for inspection
void Sixteen::digitTest(int screen, char startAt) {
  for (char i = startAt; i <= lastChar; i += 2) {
    writeChars(screen, i, i + 1);
    delay(1500);
  }
}
