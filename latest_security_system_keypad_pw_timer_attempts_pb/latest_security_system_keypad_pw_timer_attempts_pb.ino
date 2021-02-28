//*********************************************
//Brock Carroll -- MSPS
//Security System Passcode, # of Attempts, Timer, and Reset Button
//*********************************************


#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#define password_length 5
char holder[password_length];
char correctPassword[password_length] = "2003";
int state = 0;                //used for the "Enter Password" message

int buttonState = 0;          //used to hold value of push button whether it's pushed or not
int redLED = 13;              //initialize pin 13 for redLED used for testing
int greenLED = 12;            //initialize pin 12 for greenLED used for testing
int wrong_password = 0;       //used to check for the # of wrong password attempts

byte holder_count = 0;        //used to store the # of keypad inputs
char keyPress;                //used to initialize a key input from the keypad

unsigned long previous_time = 0; //stores the previous_time the timer has been counting 
unsigned long current_time = 0;  //stores the current time of the timer, which is millis
unsigned long minutes_seconds = 3;       //sets the # of minutes_seconds the timer countsdown (only 3 is used for short testing purposes)
int timerState = 0;              //stores the state of the timer to start the timer

const byte ROWS = 4;          //sets # of rows for the 4x4 keypad
const byte COLS = 4;          //sets # of columns for the 4x4 keypad

char kp[ROWS][COLS] = {       //set keypad array
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
  
};

//assign Arduino pins 9-2 for rows & columns
byte rowPins[ROWS] = {9,8,7,6};   //sets pins used for the keypad rows
byte colPins[COLS] = {5,4,3,2};   //sets pins used for the keypad columns


LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 20 chars and 4 line display
Keypad keypad = Keypad(makeKeymap(kp), rowPins, colPins, ROWS, COLS);     //creates keypad



void setup()
{
  lcd.init();                      // initialize the lcd 
  lcd.backlight();                 //turn on backlight
  pinMode(redLED, OUTPUT);         //sets output for pin 13 
  pinMode(greenLED, OUTPUT);       //sets output for pin 12
  pinMode(10, INPUT);              //sets pin 10 for push button
  digitalWrite(redLED, HIGH);      //sets redLED always HIGH @ start to replicate security system activated
  Serial.begin(9600);              //sets serial monitor baud
}


void loop(){
 
  if (state == 0){                    //"Enter Password" is only available when state is 0
    lcd.setCursor(0,0);
    lcd.print("Enter Password:");
  }
  
  char keyPress = keypad.getKey();    //sets the ability to input a key by calling the getKey function
  if (keyPress){                      //accumulates # of keypad entries. This is 4 since the holder is a function of the password length(4)
    holder[holder_count] = keyPress;
    lcd.setCursor(holder_count, 1);
    lcd.print(holder[holder_count]);
    holder_count++;
  }
  
 buttonState = digitalRead(10);       //checks for push button input
 if ((holder_count == password_length -1)||(buttonState==HIGH) || (state==1)){      //checks for the correct holder_count length of 4, the push button press, and the state = 1 where the number of entires is maximum
  if (!strcmp(holder, correctPassword) || buttonState == HIGH || (state ==1)) {     //checks if the password is correct,
    lcd.clear();
    lcd.print("Alarm Deactivated");
    delay(1000);
    timerState=0;
    timer();                  //calls the timer function to begin countdown before automatically restarting
    
    if (wrong_password >0){   //sets state back to start message to reset the # of wrong entries back to 0
      wrong_password = 0;
    }
    state = 0;
  }
  else if (strcmp(holder, correctPassword) && state == 0) {       //checks if the password is incorrect and accumulates the # of attempts
    lcd.clear();
    lcd.print("Incorrect Password");
    Serial.println("Incorrect Password");
    wrong_password++;
    wrong_pw();                                                   //calls wrong_pw function to let operator know to use the reset button
    delay(2000);
  }
 lcd.clear();                                                     //clears the lcd message & clears the keypad entry
  clearholder();
  
 }
}



void wrong_pw(){                                                  //function to check if the # of passwords is 2 (this is for testing to make things shorter; will be adjusted)
  while (wrong_password >= 2){
    buttonState = digitalRead(10);
    lcd.setCursor(0,3);
    lcd.print("Must Unlock With Key");
    if (buttonState == HIGH){                                     //checks if push button is pressed in order to reset security system
      wrong_password = 0;
      state = 1;
  }
}}

void clearholder() {                                              //clears the lcd message & clears the keypad entry
  while (holder_count != 0) {
    holder[holder_count--]= 0;
  }
 }

void timer(){                                         //timer used to begin countdown when correct password has been entered or push button has been pressed (time is only 3 minutes_seconds for testing; will be adjusted) 
  while(timerState ==0){
  char keyPress = keypad.getKey();  
  int mins = minutes_seconds/60;                      //stores the minutes value                              
  int secs = minutes_seconds % 60;                    //stores the seconds value
  current_time = millis();                            //current_time program beings
  if(timerState==0){                                  //checks if the beginning state is 0. This is allows us to change the state to get out of the specific if statements
  if((current_time-previous_time)>=1000) {            //checks the current and previous timer for 1 second. This will be how slow the timer counts down                       
  minutes_seconds--;                                  //decrements the number of seconds
  previous_time=current_time;
  lcd.clear();
  }
  lcd.setCursor(3,1);                                 //displays the countdown message with the minutes:seconds
  lcd.print("Time Remaining");
  lcd.setCursor(8,2);
  lcd.print(mins);
  lcd.print(":");
  lcd.print(secs);
  }
  
  if (timerState==0 && keyPress=='#'){                //if the '#' key is pressed, it resets the timer to 3 seconds
    timerState = 0;
    minutes_seconds = 3;
  lcd.clear();
  }

  if(timerState==0 &&keyPress=='*'){                  //if the '*' key is pressed, it resets the timer
    timerState = 1;
    state = 0;
    minutes_seconds = 3;
  }
  
  if (timerState==0 && minutes_seconds==0){           //displays end of time message and will start the reactivation process where the state is set back to 0 in the "correct password" if statement above
   lcd.clear();
   lcd.setCursor(3,1);
   lcd.print("**TIME IS UP**");
   delay(1000);
   lcd.clear();
   lcd.print("Alarm Activated");
   delay(1000);
   lcd.clear();
   timerState = 1;
   minutes_seconds=3;
   //need LEDs etc. here** will be set at a later time
   break;
  }
  }
  }

 
