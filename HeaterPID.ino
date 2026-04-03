/********************************************************
 HEATER CONTROLLER MAIN CODE FOR COMPRESSED AGRICULTURAL WASTE

 The main goal of this program is to control and display temperature of the 
 fiber preheating system. Aspects include:
 * A PID controller for the relay which switches on and off the heater
 * Thermocouple data processing (MAX6675)
 * LCD display output
 
 ********************************************************/



/* THINGS TO DO:
- configure user input for temperature setpoint using potentiometer, low priority

*/


#include <PID_v1.h> // import PID library
#include "MAX6675.h" // import MAX6675 library
#include <LiquidCrystal.h> // import library for the lcd display


// Define PID pins
#define PIN_INPUT 50 // set the PID input pin to pin 50; this is where the info from the MAX6675 comes from
#define RELAY_PIN 47 // set the PID output pin to pin 47; this is where the arduino talks to the relay


// Define the pins for the MAX6675 
const int dataPin   = 50;
const int clockPin  = 52;
const int selectPin = 49;

/*****  DONT NEED RIGHT NOW
// 4 Digit Display pin definition                              CHANGE PINS, MAKE SURE PIN PLACEMENT MATCHES THE CORRECT SEGMENTS
const uint8_t segmentPins[] = { 4, 6, 3, 10, 9, 2, 5, 13 }; // Pins to determine which segments in each digit are lit
const uint8_t digitSelectionPins[] = { 12, 11, 8, 7 }; // Pins to determine which digit is displaying
Simple5641AS component( segmentPins, digitSelectionPins );
*****/

// LCD: initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 29, en = 28, d4 = 27, d5 = 26, d6 = 25, d7 = 24;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
unsigned long lastUpdate = 0; // create a variable to run the lcd incrementally


//PID Define Variables we'll be connecting to
double Setpoint, Input, Output;



//PID Specify the links and initial tuning parameters
double Kp=8, Ki=0.1, Kd=0.1;                                // CHANGE TO TUNE PID
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

int WindowSize = 5000; // set the relay pid output to run every 5000 ms
unsigned long windowStartTime;

// Initialize the MAX6675
MAX6675 thermoCouple(selectPin, dataPin, clockPin);



void setup()  // SETUP LOOP
{
  // PID SETUP
  windowStartTime = millis(); // start the clock
  pinMode(RELAY_PIN, OUTPUT);

  //initialize the variables we're linked to
  Setpoint = 100; //                                                      CHANGE TO MAKE EDITABLE BY USER

  //tell the PID to range between 0 and the full window size
  myPID.SetOutputLimits(0, WindowSize);

  //turn the PID on
  myPID.SetMode(AUTOMATIC);



  // MAX 6675 SETUP
  Serial.begin(115200);
  Serial.println();
  Serial.println(__FILE__);
  Serial.print("MAX6675_LIB_VERSION: ");
  Serial.println(MAX6675_LIB_VERSION);
  Serial.println();
  delay(250);

  thermoCouple.begin();



  // LCD SETUP
  lcd.begin(16, 2); // set up the LCD's number of columns and rows
}

void loop()
{
  
  // MAX 6675 main loop code
  static unsigned long next_samp = 0; // define a variable to space out MAX6675 samples
  if(millis() - next_samp > 300)
  {
    next_samp = millis();
    int status = thermoCouple.read();
    if (status != STATUS_OK)
    {
      Serial.println("ERROR!");
    }
    Input = thermoCouple.getCelsius(); // get the current thermocouple temperature reading
    Serial.println(thermoCouple.getCelsius()); // print the current reading in the terminal
  }
  
  
  
  // PID main loop code
  myPID.Compute();

  //turn the output pin on/off based on pid output
  if (millis() - windowStartTime > WindowSize)
  { //time to shift the Relay Window
    windowStartTime += WindowSize;
  }
  if (Output < millis() - windowStartTime) digitalWrite(RELAY_PIN, HIGH);    // CHAT SAYS THIS LOGIC IS BACKWARDS
  else digitalWrite(RELAY_PIN, LOW);


  // LCD display, WORKING 
  if (millis() - lastUpdate > 2000) {   // update every 2000ms
    lastUpdate = millis();


    lcd.setCursor(0,0);  // Write in the first row
    lcd.print("STPNT TEMP STATS");
    

    lcd.setCursor(0, 1); // Write in the second row
    lcd.print(Setpoint, 0);
    lcd.print(" C ");
    lcd.print(Input, 0);
    lcd.print("  WAIT ");
  }

}


