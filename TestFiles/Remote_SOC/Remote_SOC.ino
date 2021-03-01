#include <TimerOne.h> //interrupt timer to check 
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;

//Current sensor veriables
#define ChargingCurrentPin A1
#define InverterCurrentPin A2
#define analogA0 A0 // voltage sensor if needed
const float Vcc    = 5.05;// supply voltage 5V or 3.3V
const float factor = 0.041;// 40mV/A is the factor(Sensativity)

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


void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  Timer1.initialize(10000000);// Sets the periodic interrupt timer to 10 seconds
  Timer1.attachInterrupt(current_check); // after 10 second has pass it will jump to the function check status of the battery.
  pinMode(buttonPin, INPUT);
  pinMode(RemotePin, OUTPUT);
  // pinMode(pinCS, OUTPUT); //pin designated to wirte to SDcard?

  while (!Serial) {
    // will pause Zero, Leonardo, etc until serial console opens
    delay(1);
    uint32_t currentFrequency;
  }
  Serial.println("Hello!");

  // Initialize the INA219.
  if (! ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) {
      delay(10);
    }
  }
  //sensativity of smaller current sensor
  ina219.setCalibration_16V_400mA();

  Serial.println("Measuring voltage and current with INA219 ...");
}

void loop() {
}
void current_check(){
  RemoteInverterControl(InverterCurrentReading());

  Serial.print("Button:           "); Serial.print(digitalRead(buttonPin)); Serial.println();
  Serial.print("Button-Prolonged: "); Serial.print(PowerButtonProlongedSignal); Serial.println();
  Serial.print("Remote on/off:    "); Serial.println(RemoteOnOff);

  SOCcapCalculation();

  Serial.print("chargin Current:      "); Serial.print(ChargingCurrentReading()); Serial.println("A");
  Serial.print("inverter Current:     "); Serial.print(InverterCurrentReading()); Serial.println("A");
  Serial.print("Control Current:      "); Serial.print(ControlSysCurrent()); Serial.println("mA");
  Serial.print("Capacity:             "); Serial.print(Capacity); Serial.println(" mAs");
  Serial.print("SOCcap:               "); Serial.print(SOCcap, 3); Serial.println("%");


  Serial.println();



  // wait 3 seconds before the next loop for the analog-to-digital
  /*delay(3000);*/
}

//In this function we used the current going to inverte, power push button signal, and the SOC to dictate the state of the remote on/off signal to inverter
void RemoteInverterControl(float InverterCurrent) {
  PowerPushButton();

  if (SOCcap >= 25 && PowerButtonProlongedSignal == true) {
    // turn inverter on:
    digitalWrite(RemotePin, HIGH);
    RemoteOnOff = true;

  } else if (SOCcap >= 25 && PowerButtonProlongedSignal == false && InverterCurrent > 0.35) {
    digitalWrite(RemotePin, HIGH);
    RemoteOnOff = true;
  } else {
    digitalWrite(RemotePin, LOW);
    RemoteOnOff = false;
  }
}


void PowerPushButton() {
  if (digitalRead(buttonPin) == HIGH) {
    PowerButtonProlongedSignal = true;
    PowerButtonTimeMark = millis();
  }

  if (PowerButtonProlongedSignal == true) {
    if ( (millis() - PowerButtonTimeMark > 30000) || (millis() - PowerButtonTimeMark < 0)) { // if statement that checks for 1 minute of delay. Also check rollover of millis
      PowerButtonProlongedSignal = false;
    }
  }
}



float ControlSysCurrent() {
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

float InverterCurrentReading() {
  double temp =   (5 / 1023.0) * (analogRead(ChargingCurrentPin)); // Read the voltage from sensor, the 4.55 is for calibration of the ADC
  //Serial.print(temp, 3); Serial.println("temp");
  //Serial.print( 4.55 + analogRead(AnalogCurrentSensorPin), 3); Serial.println("analogRead");
  float temp2 =  temp - (Vcc / 2); // fix value for quasient output voltage
  float current = temp2 / factor;

  //covers case when inverter is completely off
  if ( RemoteOnOff == false) {
    current = 0; //we can add the quiescient current to voltage sensor if we really need too (uA)
  }
  return current;
}

float ChargingCurrentReading() {
  double temp =   (5 / 1023.0) * ( 2 + analogRead(InverterCurrentPin)); // Read the voltage from sensor, the 4.55 is for calibration of the ADC
  //Serial.print(temp, 3); Serial.println("temp");
  //Serial.print( 4.55 + analogRead(AnalogCurrentSensorPin), 3); Serial.println("analogRead");
  float temp2 =  temp - (Vcc / 2); // fix value for quasient output voltage
  float current = temp2 / factor;

  //if statement secures that we only read chaging current from this sensor
  if (current <= 0.1) {
    current = 0;
    ChargingBattery = false;
  } else {
    ChargingBattery = true;
  }
  current = current * 0.95; // this accounts for the inefficiency of the battery
  return current;
}

void SOCcapCalculation() {
  if (millis() - previousCurrentReadingTime >= CurrentReadingInterval) {
    float TotalCurrentSec = (1000 * (ChargingCurrentReading() - InverterCurrentReading()) - ControlSysCurrent()) * 10; //Sum of all currents for 10 seconds
    Capacity = Capacity + TotalCurrentSec; // we still need to account for current(power consummed by MPPT if there is any)
    SOCcap = (100 * Capacity ) / 1008000000;
    previousCurrentReadingTime = millis();
  }

  //Serial.print("chargin Current? "); Serial.print(ChargingCurrentReading()); Serial.println(" ");
  //Serial.print("Control Current: "); Serial.println(ControlSysCurrent());
  //Serial.print("Capacity:       "); Serial.print(Capacity); Serial.println(" mAs");
  //Serial.print("SOCcap:         "); Serial.print(SOCcap,3); Serial.println("%");
}

void calibration() {
  /*
     need to implement counter that will keep of the time(4hours) of last major discharge or charge
     so we can use the "open voltage" to calibrate the capacity and SOCcap
  */
}
