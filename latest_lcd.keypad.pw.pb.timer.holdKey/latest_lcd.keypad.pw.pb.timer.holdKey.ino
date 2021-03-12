//*********************************************
//Brock Carroll -- MSPS
//Security System Passcode, # of Attempts, Timer, and Reset Button
//*********************************************

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#define password_length 5                           //define the password length (5 - 1 = 4)
char holder[password_length];                       //function of the password length that holds the keypad entries
char correctPassword[password_length] = "2003";     //function of the password length that holds the correct password
int state = 0;                                      //used for the "Enter Password" message
int buttonState = 25;                               //used to hold value of push button whether it's pushed or not
int redLED = 22;                                    //initialize pin 13 for redLED used for testing
int greenLED = 23;                                  //initialize pin 12 for greenLED used for testing
int wrong_password = 0;                             //used to check for the # of wrong password attempts
byte holder_count = 0;                              //used to store the # of keypad inputs
char keyPress;                                      //used to initialize a key input from the keypad
bool btn = false;                                   //used to check if the pushbutton has been pressed
unsigned long previous_time = 0;                    //stores the previous_time the timer has been counting 
unsigned long current_time = 0;                     //stores the current time of the timer, which is millis
unsigned long minutes_seconds = 6;                  //sets the # of minutes_minutes_seconds the timer countsdown (only 3 is used for short testing purposes)
int timerState = 0;                                 //stores the state of the timer to start the timer

const byte ROWS = 4;                                //sets # of rows for the 4x4 keypad
const byte COLS = 4;                                //sets # of columns for the 4x4 keypad

//sets the keypad array
char kp[ROWS][COLS] = {                             
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
  
};

// set the LCD address to 0x27 for a 20 chars and 4 line display
LiquidCrystal_I2C lcd(0x27,20,4);                   

//assign Arduino pins 53-46 for the keypad rows & columns
byte rowPins[ROWS] = {46,47,48,49};                 //sets pins used for the keypad rows
byte colPins[COLS] = {50,51,52,53};                 //sets pins used for the keypad columns

//create the keypad object
Keypad keypad = Keypad(makeKeymap(kp), rowPins, colPins, ROWS, COLS);



void setup()
{
  lcd.init();                                       // initialize the lcd 
  lcd.backlight();                                  //turn on backlight
  pinMode(redLED, OUTPUT);                          //sets output for pin 23
  digitalWrite(redLED, HIGH);                       //sets redLED always HIGH @ start to replicate security system activated 
  pinMode(greenLED, OUTPUT);                        //sets output for pin 22
  pinMode(buttonState, INPUT);                      //sets pin 25 for push button
  keypad.addEventListener(checkHold);             //adds the keypadEventListener method
  keypad.setHoldTime(5000);                         //the number of seconds the '*' key must be held
  Serial.begin(9600);                               //sets serial monitor baud
  char keyPress = keypad.getKey();
  if (state==0){
    Serial.println("Welcome to the MSPS");
    
  }
  if (state==1){
    Serial.println("Enter Password:");
  }
}

void loop(){
  
  char keyPress = keypad.getKey();                  //sets key input from the keypad
  
  if (state == 0){                                  //prints the welcome message only if the state is 0
    lcd.setCursor(0,0);
    lcd.print("--------------------");
    lcd.setCursor(0,1);
    lcd.print("Welcome to the MSPS");
    lcd.setCursor(0,2);
    lcd.print("--------------------");
    lcd.setCursor(0,3);
    lcd.print("*-*-*-*-*-*-*-*-*-*-");
    if(keyPress!=NO_KEY){
      Serial.println(keyPress);
    }
  }
  
  /*
   Eventually need something here for the LED
   else if (state == ledState){**********************************************
   ..take from char = keyPress down to the LED portion on 'don't delete' file
   }
   */

/*
the operator can now enter a password if the state 
is 1 which is determine/set by the keypadEvent function
when the '*' key is held for 5 seconds. Only the operator
should know this
*/
 if (state == 1){
  lcd.setCursor(0,0);                                   
  lcd.print("Enter Password: ");
  if(keyPress!=NO_KEY){
      Serial.println(keyPress);
    }
  if (keyPress){
    holder[holder_count] = keyPress;
    lcd.setCursor(holder_count, 1);
    lcd.print(holder[holder_count]);
    holder_count++;
  }

/*checks if the entires in the holder count matches the password length or the button has been pushed. If
any of these are true, it then checks if the password is correct or skips this if the previous if statement
had the button pushed. Either case deactivates the alarm with the corresponding message and then starts the
timer. It also resets the wrong password attemtps so there are none remaining if correct password has been
entered correctly after a false attempt.
*/ 
 buttonState = digitalRead(25);                     //reads push button state of pin 25 to check if high or low 
 if ((holder_count == password_length -1) || (buttonState==HIGH) || (btn==true)){
  lcd.clear();
  buttonState = digitalRead(25);
  if (!strcmp(holder, correctPassword) || buttonState == HIGH || (btn==true)) {
    lcd.print("Alarm Deactivated");
    timerState=0;
    timer();
    buttonState = digitalRead(25);
    if (wrong_password >0){
      wrong_password = 0;
    }
    state = 0;
    btn = false;
  }
  
  //checks if the password entered is incorrect. If so, it increments the # of wrong attempts and calls
  //the wrong_pw function
  else if (strcmp(holder, correctPassword)) {
    lcd.print("Incorrect Password");
    Serial.println("Incorrect Password");
    wrong_password++;
    wrong_pw();
    delay(2000);
  }
 lcd.clear();       //clears the LCD
  clearholder();    //calls the clearHolder function to erase the stored key inputs
  
 }
}
}

/*function to check the number of wrong passcode entries. If >= 2, then the corresponding message
appears and will stay until the buttonState is high (push button pressed), which returns to the
deactivation if statement above
*/
void wrong_pw(){
  while (wrong_password >= 2){
    buttonState = digitalRead(25);
    lcd.setCursor(0,3);
    lcd.print("Must Unlock With Key");
    if (buttonState == HIGH){
      wrong_password = 0;
      btn = true;
  }
}}

//a function that clears the holder variable that carries the keypresses
void clearholder() {
  while (holder_count != 0) {
    holder[holder_count--]= 0;
  }
 }

//special event in the keypad library that allows the holding of a keypad entry
 void checkHold(KeypadEvent keyPress){
    switch (keypad.getState()){
    case HOLD:
        if (keyPress == '*') {        //the key to be held to unlock the password availability for the operator
            state = 1;                //sets state to 1 to exit the welcome screen
            lcd.clear();
            Serial.println("Enter Password:");
        }
        break;
    }
}

/*timer function to start immediately once the security system is deactivate. Its purpose
 * is to start the security system automatically once the timer reaches 0 if the operator 
 * forgets to rearm it or whatever the reason may be. It should be set for 10 minutes, but
 * it is only for 5 seconds here for testing purspose and will be changed near the end of
 * completion. There is an option to reset the timer or end the timer and reactivate the
 * security system with various key presses.
 */

void timer(){
  while(timerState ==0){
  digitalWrite(redLED,LOW);
  digitalWrite(greenLED,HIGH);
  char keyPress = keypad.getKey();                                //check key press
  int mins = minutes_seconds/60;                                  //stores the minutes value
  int secs = minutes_seconds % 60;                                //stores the seconds value
  current_time = millis();                            //current_time program beings
  if(timerState==0){                                  //checks if the beginning state is 0. This is allows us to change the state to get out of the specific if statements
  if((current_time-previous_time)>=1000) {            //checks the current and previous timer for 1 second. This will be how slow the timer counts down                       
  minutes_seconds--;                                  //decrements the number of seconds
  previous_time=current_time;                         //sets the previous time to whatever millis is at that instance
  lcd.clear();
  }
  lcd.setCursor(3,1);                                  //displays the countdown message with the minutes:seconds
  lcd.print("Time Remaining");
  lcd.setCursor(8,2);
  lcd.print(mins);
  lcd.print(":");
  lcd.print(secs);
  Serial.println(secs);
  //delay(1000);
  }
  //delay(1000);
  if (timerState==0 && keyPress=='#'){                //if the '#' key is pressed, it resets the timer to 5 seconds
    Serial.println('#');
    timerState = 0;
    minutes_seconds = 5;
    Serial.println(minutes_seconds);
    lcd.clear();
  }

  if(timerState==0 &&keyPress=='*'){                  //if the '*' key is pressed, it resets the timer
    timerState = 1;
    state = 0;
    minutes_seconds = 5;
    Serial.println('*');
    Serial.println("Alarm Activated");
    //Serial.println(minutes_seconds);
    Serial.println("Enter Password:");
    //delay(1000);
    digitalWrite(redLED, HIGH);                       //activates security system with redLED HIGH & greenLED LOW
    digitalWrite(greenLED,LOW);                       
    lcd.clear();
  }
  
  if (timerState==0 && minutes_seconds==0){           //displays end of time message and will start the reactivation process where the state is set back to 0 in the "correct password" if statement above
   lcd.clear();
   lcd.setCursor(3,1);
   lcd.print("**TIME IS UP**");
   Serial.println("**Time Is Up**");
   digitalWrite(redLED,HIGH);                         //activates security system with redLED HIGH & greenLED LOW
   digitalWrite(greenLED,LOW);
   delay(2000);
   lcd.clear();
   delay(2000);
   lcd.print("Alarm Activated");
   Serial.println("Alarm Activated");
   timerState = 1;
   minutes_seconds=6;
   lcd.clear();
   break;
  }
  
  }
  }
