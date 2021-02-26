//Brock Carroll_MSPS
//keypad w/ password & LCD using I2C protocol

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#define password_length 5
char holder[password_length];
char correctPassword[password_length] = "2003";
byte holder_count = 0;
char keyPress;


const byte ROWS = 4;
const byte COLS = 4;

char kp[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

//assign Arduino pins 9-2 for rows & columns
byte rowPins[ROWS] = {9,8,7,6};
byte colPins[COLS] = {5,4,3,2};


LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 20 chars and 4 line display
Keypad newKeypad = Keypad(makeKeymap(kp), rowPins, colPins, ROWS, COLS);



void setup()
{
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
}


void loop(){
  
  lcd.setCursor(0,0);
  lcd.print("Enter Password:");
  
  keyPress = newKeypad.getKey();

  if (keyPress && (keyPress!='#')){
    holder[holder_count] = keyPress;
    lcd.setCursor(holder_count, 1);
    lcd.print(holder[holder_count]);
    holder_count++;}
 
 if (holder_count == password_length -1){
  lcd.clear();

  if (!strcmp(holder, correctPassword)) {
    lcd.print("Alarm Deactivated");
    delay(5000);
    
  }
  else {
    lcd.print("Incorrect");
    delay(1000); 
  }

  lcd.clear();
  clearData();
 }
}

void clearData() {
  while (holder_count != 0) {
    holder[holder_count--]= 0;
  }
 }
