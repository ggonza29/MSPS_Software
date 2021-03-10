//****************
/*modification
   George Gonzalez
   modified brock security system
   to power off and power down the security system.
*/
/*
*****************************/
#include <TimerOne.h> // calls the library for the periodic interrupt timer

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

#define half_an_hour 180000        //1800000 this is the tiem 30 min for the security system to be on but first testing it with 3 minute
#define alert_before_lcd_off 60000 //300000 first esting with one minute then change it to 5 min
#define OneMinute 60000            // one minute the LCD should be off after one minute of no user interaction

//Brock code from security keypad

#define password_length 5

char holder[password_length]; //keeps the password
char correctPassword[password_length] = "2003";

int buttonState = 0;    //used to hold value of push button whether it's pushed or not
int redLED = 13;        //initialize pin 13 for redLED used for testing
int greenLED = 12;      //initialize pin 12 for greenLED used for testing
int wrong_password = 0; //used to check for the # of wrong password attempts

byte holder_count = 0; //used to store the # of keypad inputs
char keyPress;         //used to initialize a key input from the keypad

unsigned long previous_time = 0;    //stores the previous_time the timer has been counting
unsigned long current_time = 0;     //stores the current time of the timer, which is millis
unsigned long minutes_seconds = 30; //sets the # of minutes_seconds the timer countsdown (only 3 is used for short testing purposes)
int timerState = 0;                 //stores the state of the timer to start the timer

const byte ROWS = 4; //sets # of rows for the 4x4 keypad
const byte COLS = 4; //sets # of columns for the 4x4 keypad

char kp[ROWS][COLS] = { //set keypad array
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}

};

int currentState = 0;
/*different state: state 0 - lcd off security system on; state 1 - LCD on waiting for security pin; state 2 - security system is off;
* state 3 - keypad is lock due to 3 wrong attempts;
*/
int LCD_off = 0;
int reset_timer = 0;
int mins;
int secs;
unsigned long pw_timer;          //used to display the amount of time the user needs to wait for the wrond pasword display to end
unsigned long timeStarted;       //used to check how much time passed between keypress
unsigned long oneMinute = 60000; //one minte in millis()

//assign Arduino pins 9-2 for rows & columns
byte rowPins[ROWS] = {9, 8, 7, 6}; //sets pins used for the keypad rows
byte colPins[COLS] = {5, 4, 3, 2}; //sets pins used for the keypad columns

LiquidCrystal_I2C lcd(0x27, 20, 4);                                   // set the LCD address to 0x27 for a 20 chars and 4 line display
Keypad keypad = Keypad(makeKeymap(kp), rowPins, colPins, ROWS, COLS); //creates keypad
//
void setup()
{
    lcd.init();                 //Initialize the LCD but back light is off so it appears off
    lcd.clear();                //to verify that no values are present in the LCD on setup
    pinMode(redLED, OUTPUT);    //sets output for pin 13
    pinMode(greenLED, OUTPUT);  //sets output for pin 12
    pinMode(10, INPUT);         //sets pin 10 for push button
    digitalWrite(redLED, HIGH); //sets redLED always HIGH @ start to replicate security system activated
    Serial.begin(9600);
    Timer1.initialize(5000000);         // Sets the periodic interrupt timer to 10 seconds
    Timer1.attachInterrupt(checkStatus); // after 10 second has pass it will jump to the named function and check the current of the system.
}

void loop()
{
    keyPress = keypad.getKey();    //gets kep press
    buttonState = digitalRead(10); // checks for reset
    switch (currentState)          //goes through all 5 state of the security system keypad
    {
    case 0: //The security system is off and the LCD is off

        lcd.clear();
        lcd.noBacklight();
        if (keypad.getKey() || keyPress)
        { //checking if the state we are in off LCD off and we have

            timeStarted = millis();
            currentState = 1;
            holder_count = 0;
            clearholder();
        }
        break;
    case 1: //The user is attempting to turn off the security system
        //after the two second from the incorrect password we are allowed to receive new keys inputs

        lcd.backlight(); //sets the backlight of the LCD to be visible
        lcd.setCursor(0, 0);
        lcd.print("Enter Password:");
        if (keyPress)
        {                                    //if we are in LCD on state waiting for keyinput
            holder[holder_count] = keyPress; //If  valid key is enterd and print it on the lcd
            lcd.setCursor(holder_count, 1);
            lcd.print(holder[holder_count]);
            holder_count++;
            timeStarted = millis(); //resets the time for when the LCD should turn off since it received a user input
        }

        //checks if the correct password was enter
        //note: buttonState == HIGH may be place outside the switch if we want to reset the system everytime the button is high no matter what.

        if (!strcmp(holder, correctPassword) || buttonState == HIGH)
        {
            lcd.clear();
            clearholder();
            lcd.print("Alarm Deactivated");
            timerState = 0;
            wrong_password = 0; //sets state back to start message to reset the # of wrong entries back to 0
            currentState = 5;
            pw_timer = millis();
        }
        else if (strcmp(holder, correctPassword) && holder_count == 4)
        { //checks if the password is incorrect and accumulates the # of attempts
            //Serial.println("Incorrect Password");

            wrong_password++;
            wrong_pw(); //calls wrong_pw function to let operator know to use the reset button
        }
        else if (oneMinute <= (millis() - timeStarted)) //once one minute has passed with no keypad input the LD should turn off
        {
            lcd.noBacklight();
            currentState = 0;
            lcd.clear();
            clearholder();
        }

        break;
    case 2: //the security system is deactivated
        timer();
        if (keyPress == '#') // using '#' to reset the deactivated security timer
        {
            minutes_seconds = 60;
            reset_timer = 0;
            LCD_off = 1;
            pw_timer = millis();
            current_time = millis(); //current_time program beings
            reset_timer = 1;
            timer();
        }
        else if (keyPress == '*') // using the '*' for the system should reactivate the security system
        {
            lcd.clear();
            lcd.backlight();
            LCD_off = 0;
            lcd.setCursor(3, 1);
            lcd.print("Alarm Activated");
            pw_timer = millis();
            currentState = 6;
        }
        else if (keyPress)
        {
            timer();
            pw_timer = millis();
            LCD_off = 1;
        }
        else if (LCD_off == 1 && (10000 < (millis() - pw_timer)))
        {
            lcd.noBacklight();
            lcd.clear();
            LCD_off = 0;
        }
        break;
    case 3:                      //The security is lock due to 3 continuous wrong attempts
        if (buttonState == HIGH) //if the external reset button is press it will deactivate the security system
        {
            lcd.clear();
            clearholder();
            lcd.print("Alarm Deactivated");
            timerState = 0;
            wrong_password = 0; //sets state back to start message to reset the # of wrong entries back to 0
            currentState = 5;   //move to the deactivate state
        }
        else if (keyPress) //if the keypad is press it will display the lock screen waiting for key
        {
            wrong_pw();
        }
        else if (3000 < (millis() - pw_timer))
        {
            lcd.noBacklight();
            lcd.clear();
        }
        break;
    case 4: //intermediate state to wait for the timer to run out before attempting the security code again
        if (2000 < (millis() - pw_timer))
        { //making sure we display the wrong password was enter for 2 second
            lcd.clear();
            currentState = 1;
        }
        break;
    case 5: //intermediate state for the system to acknowledge the user has deactivated the security system
        if (1000 < (millis() - pw_timer))
        {
            lcd.clear();
            currentState = 2;
            reset_timer = 1;
            minutes_seconds = 60; //sets the allotted time
            reset_timer = 0;
            LCD_off = 1;             //turns on the LCD for the timer()
            pw_timer = millis();     //keep track for how long we want the LCD to be on
            current_time = millis(); //current_time program beings
            timer();                 //calls the timer function to begin countdown before automatically restarting
        }
        break;

    case 6: //intermediate state for when the system has activated it's security system and turning LCD off
        if (3000 < (millis() - pw_timer))
        {
            currentState = 0;
        }
        break;
    }
}

void checkStatus()
{
    Serial.println("5 Seconds has pass");
}

void wrong_pw()
{ //function to check if the # of passwords is 2 (this is for testing to make things shorter; will be adjusted)
    pw_timer = millis();
    lcd.clear();
    lcd.backlight();
    lcd.backlight();
    clearholder();
    lcd.print("Incorrect Password");
    currentState = 4;
    if (wrong_password > 2)
    {
        currentState = 3;
        lcd.setCursor(0, 3);
        lcd.print("Must Unlock With Key");
        if (buttonState == HIGH)
        { //checks if push button is pressed in order to reset security system
            wrong_password = 0;
        }
    }
}

void clearholder()
{ //clears the lcd message & clears the keypad entry
    while (holder_count != 0)
    {
        holder[holder_count--] = 0;
    }
}

void timer()
{

    //checks if the beginning state is 0. This is allows us to change the state to get out of the specific if statements
    if ((millis() - current_time) > 1000)
    { //checks the current and previous timer for 1 second. This will be how slow the timer counts down

        minutes_seconds--; //decrements the number of seconds

        current_time = millis();
    }
    mins = minutes_seconds / 60; //stores the minutes value
    secs = minutes_seconds % 60; //stores the seconds value

    if (LCD_off == 1)
    {
        lcd.backlight();
        lcd.setCursor(3, 1); //displays the countdown message with the minutes:seconds
        lcd.print("Time Remaining");
        lcd.setCursor(8, 2);
        lcd.print(mins);
        lcd.print(":");
        if (secs > 9)
        {
            lcd.print(secs);
        }
        else
        {
            lcd.print("0");
            lcd.print(secs);
        }
    }
    if (minutes_seconds <= 0)
    {
        lcd.backlight();
        lcd.setCursor(3, 1);
        lcd.print("**TIME IS UP**");
        lcd.setCursor(3, 2);
        lcd.print("Alarm Activated");
        currentState = 6;
        pw_timer = millis();
        return;
    }
    return;
}
