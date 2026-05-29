/*
EMS Lab 2 Group 2
main file that get uploaded to the arduino for final
*/
#include <Arduino.h>
#include <adxl.h>
#include <selfTest.h>
#include <stepCounter.h>
#include <calibration.h>
#include <pacingID.h>

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
pacingID pacingIDInstance(accel);

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
int currentCase = 0; // Set case = 0 to initialise system to starting profile
int previousCase = 0; //Used for extra handling
int subState = 0; //this is used for handling smaller sub states in the main switch cases

//debugging
bool printed = false;

//st pin stuff
int axisWorking[3] = {0,0,0};


//calibration
int offset[3] = {0,0,0};
bool calibrated = false;
//step

int lastValue = -1;

//pace

int lastPace = 0;



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
    if(currentCase >= 0 && currentCase < 2){
      previousCase = currentCase; // Update previous case before changing
      currentCase = currentCase + 1;
    }
    else if(currentCase == 2){
      previousCase = 2;
      currentCase = 0;
    }
    // EVERY TIME you change case, reset these!
    printed = false; 
    subState = 0;    
    tft.fillScreen(ST77XX_BLACK);
  }

  else if (previousPressed == true){
    if(currentCase > 0 && currentCase <= 2){
      previousCase = currentCase; // Update previous case before changing
      currentCase = currentCase - 1;
    }
    else if(currentCase == 0){
      previousCase = 0;
      currentCase = 2;
    }
    // EVERY TIME you change case, reset these!
    printed = false; 
    subState = 0;   
    tft.fillScreen(ST77XX_BLACK); 
  }

}



void setup() {
  Serial.begin(9600); 
  pinMode(nextButton, INPUT);
  pinMode(actionButton, INPUT); 
  pinMode(previousButton, INPUT); 

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);

  digitalWrite(13, HIGH);

  //TFT setup

  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(3);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);  // bg color auto-erases old text
}

void loop() {
  stateHandling();
  actionPressed = readButtonDebounced(actionButton, actionStableState, actionLastReading, actionLastChangeTime);

  switch(currentCase){
    case 0: //home
      /*
      This is the home screen, it will display the Steps, text at the bottom right corenr indicating
      what each button colour correspeonds to.
      */
      if (!printed) {

        //Steps heading
        tft.fillScreen(ST77XX_BLACK);
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        tft.setTextSize(2); //button legend size 1.2
        tft.setCursor(35, 10);
        tft.print("Steps");
        tft.drawLine(30, 25, 95, 25, ST77XX_WHITE); //Line under the text

        //Button legend
        tft.setTextSize(1); //button legend size 1

        tft.setCursor(80, 120);
        tft.print("Buttons");

        tft.setCursor(80, 130);
        tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
        tft.print("Next");
        tft.setCursor(80, 140);
        tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
        tft.print("Action");
        tft.setCursor(80, 150);
        tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
        tft.print("Back");
        printed = true;


        //pace
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        tft.setTextSize(1); //button legend size 1.2
        tft.setCursor(0, 120);
        tft.print("Pace");
      }

      {  // braces needed to declare variables inside a switch case
        //stepCounterInstance.runStepTrack(); 
        stepCounterInstance.stepCounterV2(calibrated);
        //stepCounterInstance.maxMagnitude();
      
        int rawPace = stepCounterInstance.paceID();

        // Only update 'pace' if a new window calculation actually occurred
        int pace = lastPace; // Default to previous known state
        


        int myValue = stepCounterInstance.numberOfSteps();

        if (myValue != lastValue || currentCase !=previousCase) { // Only update display if value changed or if the state has changed
          
          tft.setCursor(55, 40);  // below the "Steps:" label
          // // tft.drawLine(startX, startY, endX, endY, color);
          // tft.drawLine(30, 25, 95, 25, ST77XX_WHITE); //Line under the text

          tft.setTextSize(3); 
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.print(myValue);

          lastValue = myValue;
        }


        if (rawPace != -1) {
          pace = rawPace; 
        }
      
        if(pace != lastPace || currentCase !=previousCase){
          tft.setCursor(0, 130);
          tft.setTextSize(1); 
          if (pace == 0) {
            tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
            tft.print("Stationary");
          } 

          else if (pace == 1) {
            tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
            tft.print("Walking   ");
          } 

          else if (pace == 2) {
            tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
            tft.print("Running   ");
          } 

          else if (pace == 3) {
            tft.setTextColor(ST77XX_BLUE, ST77XX_BLACK);
            tft.print("Sprinting  ");
          }

          tft.setTextSize(1);
          tft.setCursor(55, 70); // Just a guess for coordinates, adjust as needed
          tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
          tft.print(pacingIDInstance.getDynamicAccel()); 

          lastPace = pace;
        }
        previousCase = currentCase;
      }
     
      

    break;


    case 1: // Self test Routine

      // subState 0: Entry prompt for X
      if(subState == 0){
        if(!printed){
          tft.fillScreen(ST77XX_BLACK);
          
          // Header
          tft.setTextSize(2);
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.setCursor(0, 0);
          tft.print("Self Test");
          tft.drawFastHLine(0, 20, 128, ST77XX_WHITE);
          
          // Instructions
          tft.setTextSize(1);
          tft.setCursor(0, 28);
          tft.println("Place device with");
          tft.println("arrow pointing to");
          tft.setTextColor(ST77XX_RED, ST77XX_BLACK); // Highlight the axis
          tft.println("X axis.");
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.println("");
          tft.println("Then press action.");
          
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
          
          tft.fillScreen(ST77XX_BLACK);
          tft.setTextSize(2);
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.setCursor(0, 0);
          tft.print("Self Test");
          tft.drawFastHLine(0, 20, 128, ST77XX_WHITE);
          
          tft.setTextSize(1);
          tft.setCursor(0, 28);
          tft.println("Place device with");
          tft.println("arrow pointing to");
          tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK); // Highlight the axis
          tft.println("Y axis.");
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.println("");
          tft.println("Then press action.");
          
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
          
          tft.fillScreen(ST77XX_BLACK);
          tft.setTextSize(2);
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.setCursor(0, 0);
          tft.print("Self Test");
          tft.drawFastHLine(0, 20, 128, ST77XX_WHITE);
          
          tft.setTextSize(1);
          tft.setCursor(0, 28);
          tft.println("Place device with");
          tft.println("arrow pointing to");
          tft.setTextColor(ST77XX_BLUE, ST77XX_BLACK); // Highlight the axis
          tft.println("Z axis.");
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.println("");
          tft.println("Then press action.");
          
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
          
          tft.fillScreen(ST77XX_BLACK);
          tft.setTextSize(2);
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.setCursor(0, 0);
          tft.print("Results");
          tft.drawFastHLine(0, 20, 128, ST77XX_WHITE);
          
          tft.setTextSize(1);
          tft.setCursor(0, 28);
          
          // Print X result
          tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
          tft.print("X: "); 
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.println(axisWorking[0]);
          
          // Print Y result
          tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
          tft.print("Y: "); 
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.println(axisWorking[1]);
          
          // Print Z result
          tft.setTextColor(ST77XX_BLUE, ST77XX_BLACK);
          tft.print("Z: "); 
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.println(axisWorking[2]);
          
          tft.println("");
          tft.println("Press Action to");
          tft.println("restart test, or");
          tft.println("Next/Prev to exit");
          
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
      
      // subState 0: Prompt user
      if (subState == 0) {
        if (!printed) {
            tft.fillScreen(ST77XX_BLACK);

            tft.setTextSize(2);
            tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
            tft.setCursor(0, 0);
            tft.print("Calibratio");

            tft.drawFastHLine(0, 20, 128, ST77XX_WHITE);  // divider line

            tft.setTextSize(1);
            tft.setCursor(0, 28);
            tft.println("Please place tracker");
            tft.println("on a flat surface,");
            tft.println("then press action.");

            printed = true;
        }
        if (actionPressed) {
            subState = 1;
            printed = false;
        }
      }

      // subState 1: Running calibration
      else if (subState == 1) {
          if (!printed) {
            calibrated = true;
              tft.fillScreen(ST77XX_BLACK);

              tft.setTextSize(2);
              tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
              tft.setCursor(0, 0);
              tft.print("Calibratio");

              tft.drawFastHLine(0, 20, 128, ST77XX_WHITE);

              tft.setTextSize(1);
              tft.setCursor(0, 30);
              tft.print("Calibrating...");

              printed = true;
          }

          bool done = calibrationInstance.calibrateAll();

          if (done) {
              subState = 2;
              printed = false;
          }
      }


      // subState 2: Success screen
      else if (subState == 2) {
          if (!printed) {
              tft.fillScreen(ST77XX_BLACK);
              tft.setTextSize(2);
              tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
              tft.setCursor(0, 0);
              tft.print("Calibration");

              tft.drawFastHLine(0, 20, 128, ST77XX_GREEN);

              tft.setTextSize(1);
              tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
              tft.setCursor(0, 30);
              tft.println("Successful!");
              tft.println("");
              tft.println("Press action to");
              tft.println("recalibrate, or");
              tft.println("next/prev to exit.");

              printed = true;
          }

          if (actionPressed) {
              calibrationInstance.reset();
              subState = 0;
              printed = false;
          }
      }
    break;

  }
}