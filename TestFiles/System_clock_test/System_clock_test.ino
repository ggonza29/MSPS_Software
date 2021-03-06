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

unsigned long previous_time = 0;   //stores the previous_time the timer has been counting
unsigned long current_time = 0;    //stores the current time of the timer, which is millis
unsigned long minutes_seconds = 3; //sets the # of minutes_seconds the timer countsdown (only 3 is used for short testing purposes)
int timerState = 0;                //stores the state of the timer to start the timer

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
unsigned long wrong_pw_timer;    //used to display the amount of time the user needs to wait for the wrond pasword display to end
unsigned long timeStarted;       //used to check how much time passed between keypress
unsigned long oneMinute = 60000; //one minte in millis()
int passwordPlace = 0;           // used to detemine the place of the keypad password

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
    Timer1.initialize(10000000);         // Sets the periodic interrupt timer to 10 seconds
    Timer1.attachInterrupt(checkStatus); // after 10 second has pass it will jump to the named function and check the current of the system.
}

void loop()
{
    keyPress = keypad.getKey();    //gets kep press
    buttonState = digitalRead(10); // checks for reset
    switch (currentState)
    {
    case 0:
        if (keypad.getKey() || keyPress)
        { //checking if the state we are in off LCD off and we have

            lcd.clear();
            timeStarted = millis();
            currentState = 1;
            holder_count = 0;
            clearholder();
        }
        break;
    case 1:
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
        //digital read was tested to have not affect
        //checks for push button input
        //checks for the correct holder_count length of 4, the push button press, and the state = 1 where the number of entires is maximum
        if (!strcmp(holder, correctPassword) || buttonState == HIGH)
        {
            lcd.clear();
            clearholder();
            lcd.print("Alarm Deactivated");
            delay(1000);
            timerState = 0;
            timer();            //calls the timer function to begin countdown before automatically restarting
            wrong_password = 0; //sets state back to start message to reset the # of wrong entries back to 0

            currentState = 2;
        }
        else if (strcmp(holder, correctPassword) && holder_count == 4)
        { //checks if the password is incorrect and accumulates the # of attempts
            //Serial.println("Incorrect Password");

            wrong_password++;
            wrong_pw(); //calls wrong_pw function to let operator know to use the reset button
            delay(2000);
            lcd.clear();
        }
        else if (oneMinute <= (millis() - timeStarted))
        {
            lcd.noBacklight();
            currentState = 0;
            lcd.clear();
            clearholder();
        }
        break;
    case 2:
        if (keyPress == '#' || keyPress == '#')
        {
            timer();
        }
        else if (keyPress)
        {
            timer();
        }
        if (timerState == 1)
        {
            lcd.noBacklight();
            lcd.clear();
            currentState = 0;
        }
        break;
    case 3:
        if (buttonState == HIGH)
        {
            lcd.clear();
            clearholder();
            lcd.print("Alarm Deactivated");
            delay(1000);
            timerState = 0;
            timer();            //calls the timer function to begin countdown before automatically restarting
            wrong_password = 0; //sets state back to start message to reset the # of wrong entries back to 0
            currentState = 2;
        }
        else if (keyPress)
        {
            wrong_pw();
        }
        else if (3000 < (millis() - wrong_pw_timer))
        {
            lcd.noBacklight();
            lcd.clear();
        }
        break;
    }
}

void checkStatus()
{
    Serial.println("10 Second has pass");
}

void wrong_pw()
{ //function to check if the # of passwords is 2 (this is for testing to make things shorter; will be adjusted)
    wrong_pw_timer = millis();
    lcd.clear();
    lcd.backlight();
    clearholder();
    lcd.print("Incorrect Password");
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
{ //timer used to begin countdown when correct password has been entered or push button has been pressed (time is only 3 minutes_seconds for testing; will be adjusted)
    while (timerState == 0)
    {
        keyPress = keypad.getKey();
        int mins = minutes_seconds / 60; //stores the minutes value
        int secs = minutes_seconds % 60; //stores the seconds value
        current_time = millis();         //current_time program beings
        if (timerState == 0)
        { //checks if the beginning state is 0. This is allows us to change the state to get out of the specific if statements
            if ((current_time - previous_time) >= 1000)
            {                      //checks the current and previous timer for 1 second. This will be how slow the timer counts down
                minutes_seconds--; //decrements the number of seconds
                previous_time = current_time;
                lcd.clear();
            }
            lcd.setCursor(3, 1); //displays the countdown message with the minutes:seconds
            lcd.print("Time Remaining");
            lcd.setCursor(8, 2);
            lcd.print(mins);
            lcd.print(":");
            lcd.print(secs);
        }

        if (timerState == 0 && keyPress == '#')
        { //if the '#' key is pressed, it resets the timer to 3 seconds
            timerState = 0;
            minutes_seconds = 3;
            lcd.clear();
        }

        if (timerState == 0 && keyPress == '*')
        { //if the '*' key is pressed, it resets the timer
            timerState = 1;
            minutes_seconds = 3;
        }

        if (timerState == 0 && minutes_seconds == 0)
        { //displays end of time message and will start the reactivation process where the state is set back to 0 in the "correct password" if statement above
            lcd.clear();
            lcd.setCursor(3, 1);
            lcd.print("**TIME IS UP**");
            delay(1000);
            lcd.clear();
            lcd.print("Alarm Activated");
            delay(1000);
            lcd.clear();
            timerState = 1;
            minutes_seconds = 3;

            //need LEDs etc. here** will be set at a later time
            break;
        }
    }
}
