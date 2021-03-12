#include <TimerOne.h>

unsigned long int timer;
String property = "Waiting";


void setup() {
  Serial.begin(9600);
  Timer1.initialize(1000000);// the arduino clock is currently running at 1ns so for one second timer check we need to initialize the timer 1 mil for 1 sec
  Timer1.attachInterrupt(checkStatus); // after one second has pass it will jump to the function check status
  

}

void loop() {
  timer = millis();
  long rndN0 = getRandomNumber();
  delay(500);
  Serial.println(timer);
  
  
}
void checkStatus(){
   
  if(property == "Waiting"){
  property = "Checking Battery Level";
  }
  else {
  property == "Waiting";
  }
  Serial.println(property); 
  return;

}


long getRandomNumber() {
  unsigned long int specialNumber = random(5, 1000);
  specialNumber *= PI;

  specialNumber -= 5;
  return specialNumber;
}
