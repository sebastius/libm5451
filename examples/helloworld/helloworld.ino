#include <Sixteen.h>

#define DATAPIN  3
#define CLOCKPIN 2

Sixteen display = Sixteen();

void setup() {
  display.addScreen(DATAPIN, CLOCKPIN);
}

void loop() {
  display.scroll("Hello world.", 150);
  display.scroll("In Zweden vocht groepje quakers b! sexfilm.   Pack my box with five dozen liquor jugs.   Portez ce vieux bon whiskys au juge blond qui fume.", 150);
}
