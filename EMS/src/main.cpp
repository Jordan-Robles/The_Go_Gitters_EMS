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
const int nextButton = 4; // Mode = toggles between each sub routine 
const int actionButton = 3; // Next = insdie each subroutine we can flicker between different data to be displayed
const int previousButton = 2; //Action button = COnfirms mode selection, start and stop

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
int result = 0;


//calibration
int offset[3] = {0,0,0};
bool calibrated = false;
float height = 1.5f;
float lastHeightPrinted = 0.0f;

//step

int stepsTaken = 0;

int lastValue = -1;

//pace

int lastPace = 0;
int pendingPace = -1;
int paceConfirmCount = 0;
const int PACE_CONFIRM_THRESHOLD = 2; // tune this up/down to taste

//exercise

bool startPressed = false;
bool stopPressed = false;
int exerciseSteps = 0;
bool isPaused = false;
bool pausedDrawn = false;
float distance = 0.0;
//timer stuff
unsigned long timerStart = 0;
unsigned long lastTimerUpdate = 0;
unsigned long elapsedSeconds = 0;




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
  if (nextPressed == true && subState != 1 && !(currentCase == 2 && subState == 2)) { // if next is pressed and we are not in the middle of a workout (substate 1) and we are not in the results page (case 4)
    if(currentCase >= 0 && currentCase < 3){
      previousCase = currentCase; // Update previous case before changing
      currentCase = currentCase + 1;
    }
    else if(currentCase == 3){
      previousCase = 3;
      currentCase = 0;
    }
    // reset
    printed = false; 
    subState = 0;    
    tft.fillScreen(ST77XX_BLACK);
  }

  else if (previousPressed == true && subState != 1 && !(currentCase == 2 && subState == 2)){
    if(currentCase > 0 && currentCase <= 3){
      previousCase = currentCase; // Update previous case before changing
      currentCase = currentCase - 1;
    }
    else if(currentCase == 0){
      previousCase = 0;
      currentCase = 3;
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
  
  actionPressed = readButtonDebounced(actionButton, actionStableState, actionLastReading, actionLastChangeTime);
  nextPressed = readButtonDebounced(nextButton, nextStableState, nextLastReading, nextLastChangeTime);
  previousPressed = readButtonDebounced(previousButton, previousStableState, previousLastReading, previousLastChangeTime);
  stateHandling();


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

        //pace
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        tft.setTextSize(1); //button legend size 1.2
        tft.setCursor(0, 120);
        tft.print("Pace");

        printed = true;
      }

      // if(!calibrated){
      //   if(printed){
      //     tft.setTextSize(1); //button legend size 1.2
      //     tft.setCursor(0, 70);
      //     tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
      //     tft.print("Not Calibrated,");
      //     tft.setCursor(0, 80);
      //     tft.print("Press Action to");
      //     tft.setCursor(0, 90);
      //     tft.print("Calibrate");
      //   }
      // }
      // if(actionPressed){
      //   currentCase = 2; //goes to calibration 
      //   printed = false; // reset for next time
      // }

      {  // braces needed to declare variables inside a switch case
        stepCounterInstance.runStepTrack(); 
        //stepCounterInstance.stepCounterV2(calibrated);
        stepCounterInstance.maxMagnitude();
    
        stepsTaken = stepCounterInstance.numberOfSteps();
        int rawPace = pacingIDInstance.paceTracker(stepsTaken);
        //int rawPace = stepCounterInstance.paceID();

        // Only update 'pace' if a new window calculation actually occurred
        int pace = lastPace; // Default to previous known state

        if (stepsTaken != lastValue || currentCase !=previousCase) { // Only update display if value changed or if the state has changed
          
          tft.setCursor(55, 40);  // below the "Steps:" label
          // // tft.drawLine(startX, startY, endX, endY, color);
          // tft.drawLine(30, 25, 95, 25, ST77XX_WHITE); //Line under the text

          tft.setTextSize(3); 
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.print(stepsTaken);

          lastValue = stepsTaken;
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
          lastPace = pace;
        }
        previousCase = currentCase;
      }
    break;

    case 1://Exercise
    // subState 0: Prompt user
      if (subState == 0) {
        if (!printed) {
          //Steps heading
          tft.fillScreen(ST77XX_BLACK);
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.setTextSize(2); //button legend size 1.2
          tft.setCursor(20, 10);
          tft.print("Exercise");
          tft.drawLine(15, 25, 115, 25, ST77XX_WHITE); //Line under the text

          //Button legend
          tft.setTextSize(1); //button legend size 1

          tft.setCursor(80, 120);
          tft.print("Buttons");

          tft.setCursor(80, 130);
          tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
          tft.print("Next");
          tft.setCursor(80, 140);
          tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
          tft.print("Start");
          tft.setCursor(80, 150);
          tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
          tft.print("Back");
          printed = true;
        }
        if (actionPressed) {
          subState = 1;
          printed = false;
          stepsTaken = stepCounterInstance.numberOfSteps(); // snapshot
          elapsedSeconds = 0;
          lastValue = -1;
          lastPace = -1; //reseting pace for pace identification in the exercise routine
          lastTimerUpdate = millis();
          
        }
      }

      // subState 1: Running Exercise
      else if (subState == 1) {

        // --- Handle pause state at the TOP, before anything else ---
        if (isPaused) {
          if (!pausedDrawn) {
            tft.setTextSize(1);
            tft.setCursor(45, 80);
            tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
            tft.print("Paused");
            pausedDrawn = true;
          }

          // Resume on next button
          if (nextPressed) {
            tft.setCursor(45, 80);
            tft.setTextColor(ST77XX_BLACK, ST77XX_BLACK);
            tft.print("Paused"); // erase
            isPaused = false;
            pausedDrawn = false;
            lastTimerUpdate = millis();
          }

          // Stop on previous button
          if (previousPressed) {
            printed = false;
            subState = 2;
            isPaused = false;
            pausedDrawn = false;
            lastTimerUpdate = millis();
          }
          //return; // Skip the rest of the case this iteration
        }
        else{
          if (actionPressed) {
            isPaused = true;
            pausedDrawn = false;
          }

            // 2. Stop button
          if (previousPressed) {
            subState = 2;
            isPaused = false;
            pausedDrawn = false;
            printed = false;
          }
          if (!printed) {
            //Steps heading
            tft.fillScreen(ST77XX_BLACK);
            tft.drawLine(15, 30, 115, 30, ST77XX_WHITE); //Line under the text

            //timer label
            tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
            tft.setTextSize(2); //button legend size 1.2
            tft.setCursor(5, 10);
            tft.print("Time: ");
            //Button legend
            tft.setTextSize(1); //button legend size 1
            tft.setCursor(80, 120);
            tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
            tft.print("Buttons");

            tft.setCursor(80, 130);
            tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
            tft.print("Start");
            tft.setCursor(80, 140);
            tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
            tft.print("Pause");
            tft.setCursor(80, 150);
            tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
            tft.print("Stop");
            printed = true;

            //pace
            tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
            tft.setTextSize(1); //button legend size 1.2
            tft.setCursor(0, 120);
            tft.print("Pace");
          }

          {  // braces needed to declare variables inside a switch case
            stepCounterInstance.runStepTrack(); 
            stepCounterInstance.maxMagnitude();
            exerciseSteps = stepCounterInstance.numberOfSteps() - stepsTaken;
            int rawPace = pacingIDInstance.paceTracker(exerciseSteps);
            int pace = lastPace; // Default to previous known state
            distance = stepCounterInstance.distanceTravelled(height);

            if (exerciseSteps != lastValue) { // Only update display if value changed or if the state has changed
              tft.setCursor(55, 40);  // below the "Steps:" label
              tft.setTextSize(3); 
              tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
              tft.print(exerciseSteps);
              lastValue = exerciseSteps;


              //Printing distacne walked
              tft.setTextSize(1);
              tft.setCursor(55, 70); // Just a guess for coordinates, adjust as needed
              tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
              tft.print(distance, 1); // Print distance with 2 decimal places
            }
            if (rawPace != -1) {
              if(rawPace == pendingPace){
                paceConfirmCount++;
              }
              else{
                pendingPace = rawPace;
                paceConfirmCount = 1; // reset count for new candidate
              }
              if(paceConfirmCount >= PACE_CONFIRM_THRESHOLD){
                pace = pendingPace;
                paceConfirmCount = 0;
              }
            }
            if(pace != lastPace){
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
              lastPace = pace;
            }

            // Update timer every second
            unsigned long now = millis();
            if (now - lastTimerUpdate >= 1000) {
              elapsedSeconds++;
              lastTimerUpdate = now;
        
              // Print timer (overwrite previous value)
              tft.setTextSize(2);
              tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
              // Clear previous timer value
              tft.fillRect(10, 100, 80, 20, ST77XX_BLACK); // Adjust position/size as needed
              tft.setCursor(65, 10);
              tft.print(elapsedSeconds);
              tft.print("s");
            }
          
          }
          previousCase = currentCase;
        }
      }
      // subState 2: Finished Workout
      else if (subState == 2) {
        if (!printed) {
          tft.fillScreen(ST77XX_BLACK);
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.setTextSize(2);
          tft.setCursor(15, 10);
          tft.print("Results");
          tft.drawLine(30, 25, 95, 25, ST77XX_WHITE); //Line under the text

          tft.setTextSize(1);
          tft.setCursor(0, 40);  // below the "Steps:" label
          tft.print("Steps: ");
          tft.setCursor(80, 40);  // below the "Steps:" label
          tft.print(exerciseSteps);

          tft.setCursor(0, 70);  // below the "Steps:" label
          tft.print("Time: ");
          tft.setCursor(70, 70);  // below the "Steps:" label
          tft.print(elapsedSeconds);
          tft.setCursor(75, 70);
          tft.print("s");


          tft.setCursor(0, 100);  // below the "Steps:" label
          tft.print("Walked: ");
          tft.setCursor(80, 100);  // below the "Steps:" label
          tft.print(distance, 1); // Print distance with 2 decimal places
          tft.setCursor(85, 100);  // below the "Steps:" label
          tft.print("m");

          // printing what the user can do next
          /*
          tft.setTextSize(1);
          tft.setCursor(0, 130);

          tft.println("Press action to");
          tft.println("Start again, or");
          tft.println("next/prev to exit.");
          */
          printed = true;
        }
        else if (actionPressed) {
          calibrationInstance.reset();
          subState = 0;
          printed = false;
          distance = 0.0f;
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
            tft.fillScreen(ST77XX_BLACK);
            subState = 2;
            printed = false;
            //high value so that it will print the height on the next loop without needing to be refreshed
            lastHeightPrinted = -9999.0f;

          }
      }

      else if(subState == 2){
        if(nextPressed){
          height = height + 0.1f;
          printed = false;
        }
        else if(previousPressed){
          height = height - 0.1f;
          printed = false;
        }

        if (!printed) {
          tft.setTextSize(2);
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.setCursor(0, 0);
          tft.print("Height");
          tft.setCursor(0, 20);
          tft.print("Adjustment");

          //Button legend
          tft.setTextSize(1); //button legend size 1

          tft.setCursor(80, 120);
          tft.print("Buttons");

          tft.setCursor(80, 130);
          tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
          tft.print("Increase");
          tft.setCursor(80, 140);
          tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
          tft.print("Accept");
          tft.setCursor(80, 150);
          tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
          tft.print("Decrease");
          printed = true;
        }
        
        if(height != lastHeightPrinted){
          tft.setTextSize(3);
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          // Clear the previous value by drawing a filled rectangle over it so we dont have to refresh entire
          tft.fillRect(70, 50, 60, 24, ST77XX_BLACK); // Adjust width/height as needed
          tft.setCursor(40, 70);
          tft.print(height, 1); // 1 decimal place
          lastHeightPrinted = height;
        }

        if (actionPressed) {
          stepCounterInstance.sensitivityAdjustment(height);
          subState = 3;
          printed = false;
        }
      }

      // subState 2: Success screen
      else if (subState == 3) {
          if (!printed) {
              tft.fillScreen(ST77XX_BLACK);
              tft.setTextSize(2);
              tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
              tft.setCursor(0, 0);
              tft.print("Calibratio");

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


    case 3:
      // subState 0: Entry prompt
      if(subState == 0){
        if(!printed){
          tft.fillScreen(ST77XX_BLACK);
          tft.setTextSize(2);
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.setCursor(0, 0);
          tft.print("Self Test");
          tft.drawFastHLine(0, 20, 128, ST77XX_WHITE);
          tft.setTextSize(1);
          tft.setCursor(0, 28);
          tft.println("Press action to");
          tft.println("begin self test.");
          printed = true;
        }
        if(actionPressed){
          subState = 1;
          printed = false;
          selfTestInstance.resetSelfTest();
        }
      }

      // subState 1: Run all 3 axes sequentially, then show results
      else if(subState == 1){
        if(!printed){
          tft.fillScreen(ST77XX_BLACK);
          tft.setTextSize(2);
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.setCursor(0, 0);
          tft.print("Self Test");
          tft.drawFastHLine(0, 20, 128, ST77XX_WHITE);
          tft.setTextSize(1);
          tft.setCursor(0, 28);
          tft.println("Testing...");
          printed = true;
        }

        // currentAxis tracks which axis we are on
        // run the current axis test every loop
        result = selfTestInstance.selfTestData(selfTestInstance.currentAxis);

        if(selfTestInstance.selfTestComplete()){
          axisWorking[selfTestInstance.currentAxis] = result;
          selfTestInstance.resetSelfTest();
          selfTestInstance.currentAxis++;

          if(selfTestInstance.currentAxis >= 3){
            selfTestInstance.currentAxis = 0;
            subState = 2;
            printed = false;
          }
        }
      }

      // subState 2: Results
      else if(subState == 2){
        if(!printed){
          tft.fillScreen(ST77XX_BLACK);
          tft.setTextSize(2);
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.setCursor(0, 0);
          tft.print("Results");
          tft.drawFastHLine(0, 20, 128, ST77XX_WHITE);
          tft.setTextSize(1);
          tft.setCursor(0, 28);

          tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
          tft.print("X: ");
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.println(axisWorking[0] ? "Good" : "Fail");

          tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
          tft.print("Y: ");
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.println(axisWorking[1] ? "Good" : "Fail");

          tft.setTextColor(ST77XX_BLUE, ST77XX_BLACK);
          tft.print("Z: ");
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.println(axisWorking[2] ? "Good" : "Fail");

          tft.println("");
          tft.println("Press action to");
          tft.println("restart test.");
          printed = true;
        }
        if(actionPressed){
          subState = 0;
          printed = false;
        }
      }
    break;
  }
}