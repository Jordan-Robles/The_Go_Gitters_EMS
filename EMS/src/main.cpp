/*
EMS Lab 2 Group 2
main file that get uploaded to the arduino for final
*/
#include <Arduino.h>
#include <adxl.h>
#include <selfTest.h>
#include <stepCounter.h>
#include <calibration.h>

//SPI stuff
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define TFT_CS   9
#define TFT_DC   10
#define TFT_RST  8
#define TFT_MOSI A4
#define TFT_SCLK A5

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

adxl accel; // Creates the accelerometer object
selfTest selfTestInstance(accel); // Creates the selfTest instance with the accelerometer
calibration calibrationInstance(accel);
stepCounter stepCounterInstance(accel);

//Setting input pins for the buttons
const int nextButton = 2; // Mode = toggles between each sub routine 
const int actionButton = 3; // Next = insdie each subroutine we can flicker between different data to be displayed
const int previousButton = 4; //Action button = COnfirms mode selection, start and stop

//button bools
bool nextPressed = false;
bool actionPressed = false;
bool previousPressed = false;

//button debouncing

const unsigned long debounceDelay = 50;

bool nextStableState = HIGH;
bool nextLastReading = HIGH;
unsigned long nextLastChangeTime = 0;

bool actionStableState = HIGH;
bool actionLastReading = HIGH;
unsigned long actionLastChangeTime = 0;

bool previousStableState = HIGH;
bool previousLastReading = HIGH;
unsigned long previousLastChangeTime = 0;


//Setting pins for LED
const int led1 = 5; //red
const int led2 = 6; //yellow
const int led3 = 7; // green

//Setting pins for the GC9A01
const int DIN = A4;
const int CLK = A5;
const int CS = 9;
const int DC = 10;
const int RES = 8;



//Time
//static unsigned long currentTime =0;
//static unsigned long previousTime =0;

//state
int currentCase = 3; // Set case = 0 to initialise system to starting profile
int subState = 0; //this is used for handling smaller sub states in the main switch cases

//debugging
bool printed = false;

//st pin stuff
int axisWorking[3] = {0,0,0};


//calibration
int offset[3] = {0,0,0};

//step

int lastValue = -1;



bool readButtonDebounced(int pin, bool &stableState, bool &lastReading, unsigned long &lastChangeTime) {
  bool reading = digitalRead(pin);
  bool triggered = false;

  if (reading != lastReading) {
    lastChangeTime = millis();
  }

  if (millis() - lastChangeTime > debounceDelay) {
    if(reading != stableState) {
      stableState = reading;
      // Changes to HIGH or == LOW depending on if your buttons are active-high or active-low
      if(stableState == HIGH){
        triggered = true; // Only trigger ONCE on the edge
      }
    }
  }

  lastReading = reading;
  return triggered;
}


void stateHandling(){
  //This function is for swtiching states through buttons
  nextPressed = readButtonDebounced(nextButton, nextStableState, nextLastReading, nextLastChangeTime);
  previousPressed = readButtonDebounced(previousButton, previousStableState, previousLastReading, previousLastChangeTime);

  if (nextPressed == true){
    if(currentCase >= 0 && currentCase < 3){
      currentCase = currentCase + 1;
    }
    else if(currentCase == 3){
      currentCase = 0;
    }
    // EVERY TIME you change case, reset these!
    printed = false; 
    subState = 0;    
  }

  else if (previousPressed == true){
    if(currentCase > 0 && currentCase <= 3){
      currentCase = currentCase - 1;
    }
    else if(currentCase == 0){
      currentCase = 3;
    }
    // EVERY TIME you change case, reset these!
    printed = false; 
    subState = 0;    
  }
}



void setup() {
  Serial.begin(9600); 
  pinMode(nextButton, INPUT);
  pinMode(actionButton, INPUT); 
  pinMode(previousButton, INPUT); 

  //TFT setup
  tft.initR(INITR_GREENTAB);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(3);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);  // bg color auto-erases old text
}

void loop() {
  switch(currentCase){

    
    case 0: //home

      //Reading state of buttons
      actionPressed = readButtonDebounced(actionButton, actionStableState, actionLastReading, actionLastChangeTime);
      stateHandling();
      //Printing welcome stuff to Screen
      //atm its just serial prints
      if(!printed){
        Serial.println("Welcome user");
        Serial.println("Press next to go to self test");
        Serial.println("Press back to go to step tracking");
        printed = true;
      }

    break;



    case 1: // Self test Routine
      actionPressed = readButtonDebounced(actionButton, actionStableState, actionLastReading, actionLastChangeTime);
      stateHandling();

      // subState 0: Entry prompt
      if(subState == 0){
        if(!printed){
          Serial.println("Place Device with arrow pointing to X and press action to continue");
          printed = true;
        }
        if(actionPressed == true){  // Waits for user to press button!
          subState = 1;             // Move to next state
          printed = false;          // Reset print flag for the next state
        }
      }
      
      // subState 1: Read X, then prompt to set up Y
      else if(subState == 1){
        if(!printed){
          // Read X exactly once when entering this state
          axisWorking[0] = selfTestInstance.selfTestData(0); 
          Serial.println("Place Device with arrow pointing to Y and press action to continue");
          printed = true;
        }
        if(actionPressed == true){ 
          subState = 2;
          printed = false;
        }
      }

      // subState 2: Read Y, then prompt to set up Z
      else if(subState == 2){
        if(!printed){
          axisWorking[1] = selfTestInstance.selfTestData(1);
          Serial.println("Place Device with arrow pointing to Z and press action to continue");
          printed = true;
        }
        if(actionPressed == true){ 
          subState = 3;
          printed = false;
        }  
      }

      // subState 3: Read Z, then display results
      else if(subState == 3){
        if(!printed){
          axisWorking[2] = selfTestInstance.selfTestData(2);
          
          Serial.println("Results:");
          Serial.println(axisWorking[0]);
          Serial.println(axisWorking[1]);
          Serial.println(axisWorking[2]);
          Serial.println("Press Action to restart test, or Next/Prev to exit");
          printed = true;
        }
        // Wait for action to restart
        if(actionPressed == true){ 
          subState = 0;
          printed = false;
        }  
      }
    break;


    case 2: // Calibration
      actionPressed = readButtonDebounced(actionButton, actionStableState, actionLastReading, actionLastChangeTime);
      stateHandling();

      // subState 0: Prompt user
      if(subState == 0){
        if(!printed){
          Serial.println("Place Device on flat surface, and press action to start");
          printed = true;
        }
        if(actionPressed == true){
          subState = 1;
          printed = false;
        }
      }

      // subState 1: Perform calibration
      else if(subState == 1){
        bool done = calibrationInstance.calibrateAll();
        if (!printed) {
            Serial.println("Calibrating... keep device flat and still.");
            printed = true;
        }
        if (done) {
            subState = 2;
            printed = false;
        }
      }

      // subState 2: Show results and wait for exit/restart
      else if(subState == 2){
        if (!printed) {
            Serial.println("Results (offsets in g):");
            Serial.println(accel.getOffset(0));   // X
            Serial.println(accel.getOffset(1));   // Y
            Serial.println(accel.getOffset(2));   // Z
            Serial.println("Press action to recalibrate, or next/previous to exit.");
            printed = true;
        }
        if (actionPressed == true) {
            calibrationInstance.reset();
            subState = 0;
            printed = false;
        }
      }
    break;




    


    case 3: // Step tracking
      actionPressed = readButtonDebounced(actionButton, actionStableState, actionLastReading, actionLastChangeTime);
      stateHandling();

      if (!printed) {
        tft.fillScreen(ST77XX_BLACK);
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        tft.setTextSize(3);
        tft.setCursor(0, 0);
        tft.print("Steps:");
        printed = true;
      }

      {  // braces needed to declare variables inside a switch case
        stepCounterInstance.runStepTrack(); 
        int myValue = stepCounterInstance.numberOfSteps();
        Serial.println(myValue);

        if (myValue != lastValue) {
          tft.setCursor(0, 30);  // below the "Steps:" label

          if (myValue < 10)        tft.print("   ");
          else if (myValue < 100)  tft.print("  ");
          else if (myValue < 1000) tft.print(" ");

          tft.print(myValue);
          lastValue = myValue;
        }
      }
    break;

  }
}


