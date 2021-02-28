#include "LCDLib.h"

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "Arduino.h"
#include "LCDLib.h"
LiquidCrystal_I2C lcd(0x27,20,4);

screen::screen(bool displayMSg){
  
}
/*void LCDlib::screen(){
  
  
}*/
void screen::SETUP(){
  lcd.init();
  lcd.clear();
}
 /*void LCDLib::displayLCD(){
  lcd.clear();
  lcd.backlight();
  lcd.display();
  lcd.setCursor(0,0);
  lcd.print("Power Remaining:");
  lcd.setCursor(0,1);
  lcd.print("Time Remaining");
  lcd.setCursor(0,2);
  lcd.print("Shut off in 1 min");
  lcd.setCursor(0,3);
  lcd.print("Press Button");
  return;
  
}*/
