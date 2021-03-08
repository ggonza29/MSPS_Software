#include <Wire.h>
#include <Adafruit_INA219.h>
//#include <TimerOne.h> // calls the library for the periodic interrupt timer

Adafruit_INA219 ina219;

//Current sensor veriables
#define ChargingCurrentPin A3
#define InverterCurrentPin A2
#define analogA0 A0 // voltage sensor if needed
const float Vquiescient  = 2.514;// supply voltage 5V or 3.3V
const float factor = 0.04;// 40mV/A is the factor(Sensativity)


const unsigned long CurrentReadingInterval = 10000; //Every 10 seconds, we will check the state of our current senosor and use that value to update the SOC
unsigned long previousCurrentReadingTime = 0;

float Capacity = 252000000; //this represents a full batter; 280000mAh == 1008000000mAseconds. We will hardcode this value to 25% capacity
float SOCenergy = 0; //this is a reading from open voltage
float SOCcap = 0; //calculation from current readings

bool ChargingBattery = false;

// Variables that deal with voltage sensors 
int SOC = 25;


// Variables used to control REMOTE to inverter
bool RemoteOnOff = false;
bool PowerButtonProlongedSignal = false;
float PowerButtonTimeMark = 0; // variable to keep the signal from power button high for 1 minute
const int buttonPin = 2;
const int RemotePin = 3;

//Variable and libraries to log data
#include <SD.h>
#include <SPI.h>
#include <DS3231.h>
File myFile;
DS3231  rtc(SDA, SCL);
int pinCS = 53; // Pin 10 on Arduino Uno

float EnergyToUser = 0; //This is the energy that a user will get from one sitting. It will go back to zero when RemoteOnOff flag is false, but it will get recorde before that.
                       //will be kept in Joules, then it will be converter to Wh before logging.
const unsigned long energyToUserInterval = 5000; //Every 5 seconds we integrate the power delivered to user
unsigned long CounterForEnergyInterval = 0;
String startTimeOfEnergySupply;

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  pinMode(buttonPin, INPUT);
  pinMode(RemotePin, OUTPUT);
  //Timer1.initialize(5000000);         // Sets the periodic interrupt timer to 5 seconds
  //Timer1.attachInterrupt(SOC); // after 10 second has pass it will jump to the named function and check the current of the system.
  // pinMode(pinCS, OUTPUT); //pin designated to wirte to SDcard?


  
  while (!Serial) {
      // will pause Zero, Leonardo, etc until serial console opens
      delay(1);
   uint32_t currentFrequency;
  }
  // Initialize the INA219.
  if (! ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }
  //sensativity of smaller current sensor
  ina219.setCalibration_16V_400mA();


  //Lines that initialize the logging modules:SD card and real time clock
  pinMode(pinCS, OUTPUT);
  if (SD.begin())
  {
    Serial.println("SD card is ready to use.");
  } else
  {
    Serial.println("SD card initialization failed");
    return;
  }
  rtc.begin(); 

}

void loop(){
  
  RemoteInverterControl(InverterCurrentReading());

  Serial.print("Button:           "); Serial.print(digitalRead(buttonPin)); Serial.println();
  Serial.print("Button-Prolonged: "); Serial.print(PowerButtonProlongedSignal); Serial.println();
  Serial.print("Remote on/off:        "); Serial.println(RemoteOnOff);
  
  SOCcapCalculation();

  Serial.print("chargin Current:      "); Serial.print(ChargingCurrentReading()); Serial.println("A");
  Serial.print("inverter Current:     "); Serial.print(InverterCurrentReading()); Serial.println("A");
  Serial.print("Control Current:      "); Serial.print(ControlSysCurrent()); Serial.println("mA");
  Serial.print("Capacity:             "); Serial.print(Capacity); Serial.println(" mAs");
  Serial.print("SOCcap:               "); Serial.print(SOCcap,3); Serial.println("%");
  

  Serial.println();


  dataLog();
  // wait 3 seconds before the next loop for the analog-to-digital
  //delay(5000);
}

//In this function we used the current going to inverte, power push button signal, and the SOC to dictate the state of the remote on/off signal to inverter
void RemoteInverterControl(float InverterCurrent){
  PowerPushButton();
  
  if (SOCcap >= 25 && PowerButtonProlongedSignal == true){
    // turn inverter on:
    digitalWrite(RemotePin, HIGH); 
    RemoteOnOff = true;
    
  }else if(SOCcap >= 25 && PowerButtonProlongedSignal == false && InverterCurrent > 0.35) {
    digitalWrite(RemotePin, HIGH); 
    RemoteOnOff = true;
  }else{
    digitalWrite(RemotePin, LOW);
    RemoteOnOff = false;
  }
}


void PowerPushButton(){
  if(digitalRead(buttonPin) == HIGH) {
     PowerButtonProlongedSignal = true;
     PowerButtonTimeMark = millis();
  }

  if(PowerButtonProlongedSignal == true) {
    if( (millis() - PowerButtonTimeMark > 10000) || (millis() - PowerButtonTimeMark < 0)){// if statement that checks for 1 minute of delay. Also check rollover of millis
      PowerButtonProlongedSignal = false;
    }
  }
}



float ControlSysCurrent(){
  float busvoltage = 0;
  float current_mA = 0;
  float power_mW = 0;
  
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  
  
  //Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  //Serial.print("Control Current:       "); Serial.print(current_mA); Serial.println(" mA");
  //Serial.print("Power:         "); Serial.print(power_mW); Serial.println(" mW");
  //Serial.println("");
  return current_mA;
}

float InverterCurrentReading(){
  double temp =   (5 / 1023.0)* (4 + analogRead(InverterCurrentPin));// Read the voltage from sensor, the 4.55 is for calibration of the ADC
  //Serial.print(temp, 3); Serial.println("temp");
  //Serial.print( 4.55 + analogRead(AnalogCurrentSensorPin), 3); Serial.println("analogRead");
  float temp2 =  temp - Vquiescient;// fix value for quasient output voltage
  float current = temp2 / factor;

  //covers case when inverter is completely off
  if( RemoteOnOff == false || current < 0){
    current = 0; //we can add the quiescient current to voltage sensor if we really need too (uA)
  }
  return current;
}

float ChargingCurrentReading(){
  double temp =   (5 / 1023.0)* ( 4 + analogRead(ChargingCurrentPin));// Read the voltage from sensor, the 4.55 is for calibration of the ADC
  //Serial.print(temp, 3); Serial.println("temp");
  //Serial.print(analogRead(ChargingCurrentPin)); Serial.println("  analogRead");
  float temp2 =  temp - Vquiescient;// fix value for quasient output voltage
  float current = temp2 / factor;

  //if statement secures that we only read chaging current from this sensor
  if(current <= 0.1){
    current = 0;
    ChargingBattery = false;
  }else{
    ChargingBattery = true;
  }
  current = current * 0.95; // this accounts for the inefficiency of the battery
  return current;
}

void SOCcapCalculation(){
   if(millis() - previousCurrentReadingTime >= CurrentReadingInterval){
      float TotalCurrentSec = (1000 * (ChargingCurrentReading() - InverterCurrentReading()) - ControlSysCurrent()) * 10; //Sum of all currents for 10 seconds
      Capacity = Capacity + TotalCurrentSec; // we still need to account for current(power consummed by MPPT if there is any)
      SOCcap = (100 * Capacity ) / 1008000000;
      previousCurrentReadingTime = millis();
   }

   SOCcap = (100 * Capacity ) / 1008000000;

   //Serial.print("chargin Current? "); Serial.print(ChargingCurrentReading()); Serial.println(" ");
   //Serial.print("Control Current: "); Serial.println(ControlSysCurrent());
   //Serial.print("Capacity:       "); Serial.print(Capacity); Serial.println(" mAs");
   //Serial.print("SOCcap:         "); Serial.print(SOCcap,3); Serial.println("%");
}


void calibration(){
  /*
   * need to implement counter that will keep of the time(4hours) of last major discharge or charge
   * so we can use the "open voltage" to calibrate the capacity and SOCcap
   */
}

void dataLog(){
  if(RemoteOnOff == true && EnergyToUser == 0){
    startTimeOfEnergySupply = rtc.getTimeStr();
    EnergyToUser = 1;
    CounterForEnergyInterval = millis();
  }else if(RemoteOnOff == true && EnergyToUser > 0){
    if(millis() - CounterForEnergyInterval >= energyToUserInterval){
      EnergyToUser = EnergyToUser + (5 * -InverterCurrentReading() * ina219.getBusVoltage_V()); //We will use this ina219Voltga assuming it will be equal to the voltage at the positive terminal of the inverter 
      CounterForEnergyInterval = millis();
      //Serial.println("");
      //Serial.print(EnergyToUser); Serial.print("   "); Serial.print(InverterCurrentReading()); Serial.print("   "); Serial.println(ina219.getBusVoltage_V());
    }
    //Serial.println("step");
  }else if(RemoteOnOff == false &&  EnergyToUser > 0){

  //Serial.println("card initialized.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
   EnergyToUser =  0.00027778 * EnergyToUser; //conversion from Joules to wh
   myFile = SD.open("FistFile.txt", FILE_WRITE);
    if (myFile) {
       Serial.print("logging to FistFile.txt...");
       //myFile.println("testing 1, 2, 3.");
       myFile.print(startTimeOfEnergySupply); myFile.print(",       "); 
       myFile.print(rtc.getTimeStr()); myFile.print(", "); myFile.print(rtc.getDateStr()); myFile.print(",       ");
       myFile.print(EnergyToUser); myFile.print(" Wh, "); myFile.print(SOCcap); myFile.println(" SOC%"); 
       // close the file:
       myFile.close();
       Serial.println("done.");
       } else {
      // if the file didn't open, print an error:
       Serial.println("error opening FirstFile.txt");
       }
    
    //Here we would log the date
    /*
    Serial.println(EnergyToUser);
    Serial.print("logdata: ");
    
    Serial.print(startTimeOfEnergySupply); Serial.print(",       "); 
    Serial.print(rtc.getTimeStr()); Serial.print(", "); Serial.print(rtc.getDateStr()); Serial.print(",       ");
    Serial.print(EnergyToUser); Serial.print(" Joules, "); Serial.print(SOCcap); Serial.println("SOC%"); 
    */
    EnergyToUser = 0;
  }else{
    EnergyToUser = 0; //reset the energy sum to zero for new recording
  }
}
