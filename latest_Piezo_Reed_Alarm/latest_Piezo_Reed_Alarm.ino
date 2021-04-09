#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#define password_length 5                           //define the password length (5 - 1 = 4)
char holder[password_length];                       //function of the password length that holds the keypad entries
char correctPassword[password_length] = "2003";     //function of the password length that holds the correct password
int state = 0;                                      //used for the "Enter Password" message
int buttonState = 25;                               //used to hold value of push button whether it's pushed or not
int reedState = 29;
int redLED = 22;                                    //initialize pin 13 for redLED used for testing
int greenLED = 23;                                  //initialize pin 12 for greenLED used for testing
int alert = 32;                                     //initialize pin 32 for the small piezo speaker
int reed = 29;
int alarm = 10;
int wrong_password = 0;                             //used to check for the # of wrong password attempts
byte holder_count = 0;                              //used to store the # of keypad inputs
char keyPress;                                      //used to initialize a key input from the keypad
bool btn = false;                                   //used to check if the pushbutton has been pressed
bool reedSwitch = false;
int reedStart = 0;
bool unlock = false;
unsigned long previous_time = 0;                    //stores the previous_time the timer has been counting 
unsigned long current_time = 0;                     //stores the current time of the timer, which is millis
unsigned long minutes_seconds = 601;                  //sets the # of minutes_minutes_seconds the timer countsdown (only 3 is used for short testing purposes)
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
  pinMode(reedState, INPUT);                        //sets pin 29 to HIGH due to pull-up resistor
  //pinMode(alarm, OUTPUT);
  keypad.addEventListener(keypadEvent);             //adds the keypadEventListener method
  keypad.setHoldTime(2000);                         //the number of seconds the '*' key must be held
  Serial.begin(9600);                               //sets serial monitor baud
  Serial.println("Enter Password:");
  
}


void loop(){
  
  /*implement the magent & reed switch if the magnet is separated from the reed,
  the alarm (piezo for testing) will sound. This is to replicated unauthorized
  access to the battery cabinet.*/
  //will stay on until shutoff via the correct password AND door closed (reedState == HIGH)
  int reedState = digitalRead(reed);
   
  char keyPress = keypad.getKey();
  
  if (state == 0 && reedState==HIGH && reedSwitch==false){
    lcd.setCursor(0,0);
    lcd.print("--------------------");
    lcd.setCursor(0,1);
    lcd.print("Welcome to the MSPS");
    lcd.setCursor(0,2);
    lcd.print("--------------------");
    lcd.setCursor(0,3);
    lcd.print("*-*-*-*-*-*-*-*-*-*-");
  }

  else if (state==0 &&reedState==LOW &&reedSwitch==false){    //clears the welcome message
    lcd.clear();
  }

  if(reedState==LOW && (reedStart == 0 || reedStart==1) && (reedSwitch==false||reedSwitch==true)){
    //lcd.clear();
    reedStart==1;
    reedSwitch=true;
    lcd.setCursor(8,0);
    lcd.print("ALERT");
  }

  if ((state == 0||state==1) && reedSwitch==true){
    //lcd.clear();
    lcd.setCursor(8,0);
    lcd.print("ALERT");
    //digitalWrite(alarm,HIGH);  
    tone(alarm, 2000, 800);
    //lcd.clear();
    //delay(1000);
    //noTone(piezo);
    
    reedStart=1;
     
  }

  
  if (((state == 1) && reedStart==1 && reedState==HIGH) ||(state==1 && reedStart==1 && reedState==LOW)) {
  //lcd.clear();
  lcd.setCursor(3,1);
  lcd.print("Enter Password");
  delay(50);
  //if (reedState==LOW){
    //Piezo();
 // }
 
  if (keyPress){
    holder[holder_count] = keyPress;
    lcd.setCursor(holder_count + 8, 2);
    lcd.print(holder[holder_count]);
    holder_count++;
    //delay(500);
  }


/*checks if the entries in the holder count matches the password length or the button has been pushed. If
//any of these are true, it then checks if the password is correct or skips this if the previous if statement
//had the button pushed. Either case deactivates the alarm with the corresponding message and then starts the
//timer. It also resets the wrong password attemtps so there are none remaining if correct password has been
entered correctly after a flase attempt.
*/

 buttonState = digitalRead(25);                     //reads push button state of pin 25 to check if high or low 
 if (((holder_count == password_length -1)||(buttonState==HIGH) || (btn==true))){
  lcd.clear();
  if ((!strcmp(holder, correctPassword) || buttonState == HIGH || (btn==true))&& reedState==HIGH) {
    lcd.setCursor(1,1);
    lcd.print("Alarm Deactivated");
    delay(2000);
    reedSwitch = false;
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
  else if (strcmp(holder, correctPassword) && reedState==HIGH) {
    lcd.clear();
    lcd.setCursor(1,1);
    lcd.print("Incorrect Password");
    Serial.println("Incorrect Password");
    wrong_password++;
    wrong_pw();
    delay(1000);
  }

  else  if ((!strcmp(holder, correctPassword) || strcmp(holder, correctPassword) || buttonState == HIGH) && reedState==LOW) {
    lcd.setCursor(5,1);
    lcd.print("Close Door");
    wrong_password=0;
    delay(1000);
  }


 lcd.clear();        
 clearholder();    //calls the clearholder function
  
 }
}
}

//function to check the number of wrong passcode entries. If >= 2, then the corresponding message
//appears and will stay until the buttonState is high (push button pressed), which returns to the
//deactivation if statement above
void wrong_pw(){
  reedState=digitalRead(29);
  while (wrong_password == 2){
    if ((state == 0||state==1) && reedSwitch==true){
    //lcd.clear();
    lcd.setCursor(8,0);
    lcd.print("ALERT");
      
    tone(alarm, 2000, 800);
    //lcd.clear();
    //delay(1000);
    //noTone(piezo);
    
    reedStart=1;
     
  }
    reedStart=1;
    if (reedState==LOW){
      Piezo();  
    }
    buttonState = digitalRead(25);
    
    lcd.setCursor(0,2);
    lcd.print("Must Unlock With Key");
    
    if(buttonState==HIGH){
      wrong_password = 0;
      btn = true;
      //reedStart=0;
      //reedSwitch = false;
  }  
  
}}

//a function that clears the holder variable that carries the keypresses
void clearholder() {
  while (holder_count != 0) {
    holder[holder_count--]= 0;
  }
 }

//special event in the keypad library that allows the holding of a keypad entry
 void keypadEvent(KeypadEvent keyPress){
    switch (keypad.getState()){
    case HOLD:
        if (keyPress == '*') {    //the key to be held to unlock the password availability for the operator
            state = 1;            //sets state to 1 to exit the welcome screen
            lcd.clear();
            reedStart=1;
            //reedSwitch = false;
        }
        break;
    }
}

void door(){
  lcd.setCursor(0,3);
  lcd.print("Close the Door");
}

void Piezo(){
  
    //lcd.clear();
    lcd.setCursor(8,2);
    lcd.print("ALERT");
    digitalWrite(alarm,HIGH);  
    //tone(alarm, 2000, 800);
    //lcd.clear();
    //delay(1000);
    //noTone(piezo);
    
    reedStart=1;
     
  
}


void timer(){                                         //timer used to begin countdown when correct password has been entered or push button has been pressed (time is only 3 minutes_seconds for testing; will be adjusted) 
  while(timerState ==0){
  digitalWrite(redLED,LOW);
  digitalWrite(greenLED,HIGH);
  char keyPress = keypad.getKey();  
  int mins = minutes_seconds/60;                      //stores the minutes value                              
  int secs = minutes_seconds % 60;                    //stores the seconds value
  current_time = millis();                            //current_time program beings
  if(timerState==0){                                  //checks if the beginning state is 0. This is allows us to change the state to get out of the specific if statements
  if((current_time-previous_time)>=1000) {            //checks the current and previous timer for 1 second. This will be how slow the timer counts down                       
  minutes_seconds--;                                  //decrements the number of seconds
  previous_time=current_time;
  lcd.clear();
  //delay(1000);
  }
  lcd.setCursor(3,1);                                 //displays the countdown message with the minutes:seconds
  lcd.print("Time Remaining");
  lcd.setCursor(8,2);
  lcd.print(mins);
  lcd.print(":");
  lcd.print(secs);
  Serial.println(secs);
  //delay(1000);
  }
  //delay(1000);
  if (timerState == 0 && keyPress == '#'){                //if the '#' key is pressed, it resets the timer to 3 seconds
    Serial.println('#');
    timerState = 0;
    minutes_seconds = 20;
    Serial.println(minutes_seconds);
    lcd.clear();
  }

  if(timerState == 0 && keyPress == '*'){                  //if the '*' key is pressed, it resets the timer
    timerState = 1;
    state = 0;
    minutes_seconds = 21;
    lcd.clear();
    lcd.setCursor(2,1);
    lcd.print("Alarm Activated");
    delay(1700);
    digitalWrite(redLED, HIGH);                       //activates security system with redLED HIGH & greenLED LOW
    digitalWrite(greenLED,LOW);                       
    lcd.clear();
  }
  
  /*int reedState = digitalRead(reed);
  if(minutes_seconds>0 && reedState==LOW){
    tone(alert,2000,2000);
    lcd.clear();
    lcd.print("Close Door");
    minutes_seconds = 17;
  }/*/

 if (minutes_seconds<10){                            //prevents the 0 digit on 10 from staying when switching from 10 to 9
    lcd.print(' ');
    }

  if(minutes_seconds==10 && minutes_seconds>=8){
      tone(alert, 2000, 2000);
    }

    if(minutes_seconds<=4){
      tone(alert,2000,1000);
      delay(1000);
    }
  
  if (timerState==0 && minutes_seconds==-1){           //displays end of time message and will start the reactivation process where the state is set back to 0 in the "correct password" if statement above
   tone(alert,2000,1000);
   delay(1000);
   lcd.clear();
   lcd.setCursor(3,1);
   lcd.print("**TIME IS UP**");
   Serial.println("**Time Is Up**");
   digitalWrite(redLED,HIGH);                         //activates security system with redLED HIGH & greenLED LOW
   digitalWrite(greenLED,LOW);
   delay(2000);
   lcd.clear();
   lcd.setCursor(2,1);
   lcd.print("Alarm Activated");
   Serial.println("Alarm Activated");
   delay(2000);
   timerState = 1;
   minutes_seconds=21;
   lcd.clear();
   break;
  }
  
  }
  }
