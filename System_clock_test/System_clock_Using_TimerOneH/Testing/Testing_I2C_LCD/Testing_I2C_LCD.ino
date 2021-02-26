#include <TimerOne.h>
#include "LCD.h"

#include <Wire.h> 
//#include <LiquidCrystal_I2C.h>

//LiquidCrystal_I2C lcd(0x27,20,4);

#include "LCDLib.h"

screen scre(true);

void setup() {
  // put your setup code here, to run once:
  /*lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);*/
  lcdlib.SETUP();
  Serial.begin(9600);
  Timer1.initialize(10000000);// the arduino clock is currently running at 1ns so for one second timer check we need to initialize the timer 1 mil for 1 sec
  Timer1.attachInterrupt(checkStatus); // after one second has pass it will jump to the function check status
}

void loop() {
  /*lcd.setCursor(0,0);
  lcd.print("Power Remaining:");
  lcd.setCursor(0,1);
  lcd.print("Time Remaining");
   lcd.setCursor(0,2);
  lcd.print("Shut off in 1 min");
   lcd.setCursor(0,3);
  lcd.print("Press Button");*/
}

void checkStatus(){
  //lcdlib.displayLCD();
  return;
}
