#include "TestLibrary.h"
#include <TimerOne.h>

TestLibrary testlibrary(true);

void setup() {
  Serial.begin(9600);// allows to view print value
  Timer1.initialize(1000000);// the arduino clock is currently running at 1ns so for one second timer check we need to initialize the timer 1 mil for 1 sec
  Timer1.attachInterrupt(checkStatus); // after one second has pass it will jump to the function check status
  

  randomSeed(analogRead(A0));//allows to access the random numbe rfunction

}

void loop() {
  // put your main code here, to run repeatedly:

}

void checkStatus(){
  long rndNo = testlibrary.getRandomNumber();
  Serial.println(rndNo); 
  return;
}
