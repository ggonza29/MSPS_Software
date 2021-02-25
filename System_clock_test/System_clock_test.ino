#include <avr/interrupt.h>



unsigned long timeClock = 0;
unsigned long timeDifferent = 0;

void setup() {
  Serial.begin(9600);

}

void loop() {
 
 timeClock = millis();
 Serial.print(" System clock time: ");
 Serial.println(timeClock);

 timeDifferent = timeClock - millis();
 Serial.print("The time different from the clock and tracker: ");
 Serial.println(timeDifferent);

 
}
