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

adxl accel;
selfTest selfTestInstance(accel);
calibration calibrationInstance(accel);
stepCounter stepCounterInstance(accel);
pacingID pacingIDInstance(accel);

// Input pins for buttons
const int nextButton     = 4;
const int actionButton   = 3;
const int previousButton = 2;

// Button state bools
bool nextPressed     = false;
bool actionPressed   = false;
bool previousPressed = false;

// Button debouncing
const unsigned long debounceDelay = 50;

bool nextStableState      = HIGH;
bool nextLastReading      = HIGH;
unsigned long nextLastChangeTime = 0;

bool actionStableState      = HIGH;
bool actionLastReading      = HIGH;
unsigned long actionLastChangeTime = 0;

bool previousStableState      = HIGH;
bool previousLastReading      = HIGH;
unsigned long previousLastChangeTime = 0;

// LED pins
const int led1 = 5; // red
const int led2 = 6; // yellow
const int led3 = 7; // green

// State
int currentCase  = 0;
int previousCase = 0;
int subState     = 0;

// Debugging
bool printed = false;

// Self test
int axisWorking[3] = {0, 0, 0};
int result = 0;

// Calibration
int   offset[3]             = {0, 0, 0};
bool  calibrated             = false;
bool  calibrationWarningDrawn = false;
float height                 = 1.5f;
float lastHeightPrinted      = 0.0f;

// Step counter
int stepsTaken = 0;
int lastValue  = -1;

// Pace
int lastPace             = 0;
int pendingPace          = -1;
int paceConfirmCount     = 0;
const int PACE_CONFIRM_THRESHOLD = 2;

// Exercise
bool  startPressed   = false;
bool  stopPressed    = false;
int   exerciseSteps  = 0;
bool  isPaused       = false;
bool  pausedDrawn    = false;
float distance       = 0.0;

// Timer
unsigned long timerStart      = 0;
unsigned long lastTimerUpdate = 0;
unsigned long elapsedSeconds  = 0;



// === HELPER FUNCTIONS ===


/**
 * Draws the 3-button legend in the bottom-right corner.
 * Labels correspond to Green (Next), Yellow (Action), Red (Back/Previous).
 */
void drawButtonLegend(const char* greenLabel, const char* yellowLabel, const char* redLabel) {
  tft.setTextSize(1);
  tft.setCursor(80, 120);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.print("Buttons");

  tft.setCursor(80, 130);
  tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
  tft.print(greenLabel);

  tft.setCursor(80, 140);
  tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
  tft.print(yellowLabel);

  tft.setCursor(80, 150);
  tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
  tft.print(redLabel);
}

/**
 * Draws a screen title at (0,0) with a full-width horizontal divider at y=20.
 * Optionally pass a colour (defaults to white).
 */
void drawScreenHeader(const char* title, uint16_t color = ST77XX_WHITE) {
  tft.setTextSize(2);
  tft.setTextColor(color, ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.print(title);
  tft.drawFastHLine(0, 20, 128, color);
}

/**
 * Prints the current pace label at the fixed bottom-left position.
 * Trailing spaces on each string clear the previous longer label.
 */
void drawPaceLabel(int pace) {
  tft.setCursor(0, 130);
  tft.setTextSize(1);
  if (pace == 0) {
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
    tft.print("Stationary ");
  } else if (pace == 1) {
    tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
    tft.print("Walking    ");
  } else if (pace == 2) {
    tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
    tft.print("Running    ");
  } else if (pace == 3) {
    tft.setTextColor(ST77XX_BLUE, ST77XX_BLACK);
    tft.print("Sprinting  ");
  }
}

/**
 * Resets all state variables when switching between cases.
 * Clears the screen so the new case starts fresh.
 */
void resetState() {
  printed  = false;
  subState = 0;
  tft.fillScreen(ST77XX_BLACK);
}


// =============================================================================
// BUTTON DEBOUNCE
// =============================================================================

bool readButtonDebounced(int pin, bool &stableState, bool &lastReading, unsigned long &lastChangeTime) {
  bool reading   = digitalRead(pin);
  bool triggered = false;

  if (reading != lastReading) {
    lastChangeTime = millis();
  }

  if (millis() - lastChangeTime > debounceDelay) {
    if (reading != stableState) {
      stableState = reading;
      if (stableState == HIGH) {
        triggered = true;
      }
    }
  }

  lastReading = reading;
  return triggered;
}


// =============================================================================
// STATE HANDLING
// =============================================================================

void stateHandling() {
  // Block navigation during an active workout or while on the results screen
  bool locked = (subState == 1) || (currentCase == 2 && subState == 2);

  if (nextPressed && !locked) {
    previousCase = currentCase;
    currentCase  = (currentCase < 3) ? currentCase + 1 : 0;
    resetState();
  } else if (previousPressed && !locked) {
    previousCase = currentCase;
    currentCase  = (currentCase > 0) ? currentCase - 1 : 3;
    resetState();
  }
}


// =============================================================================
// SETUP
// =============================================================================

void setup() {
  Serial.begin(9600);
  pinMode(nextButton,     INPUT);
  pinMode(actionButton,   INPUT);
  pinMode(previousButton, INPUT);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);

  digitalWrite(13, HIGH);

  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(3);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
}


// =============================================================================
// MAIN LOOP
// =============================================================================

void loop() {
  Serial.println(subState);

  actionPressed   = readButtonDebounced(actionButton,   actionStableState,   actionLastReading,   actionLastChangeTime);
  nextPressed     = readButtonDebounced(nextButton,     nextStableState,     nextLastReading,     nextLastChangeTime);
  previousPressed = readButtonDebounced(previousButton, previousStableState, previousLastReading, previousLastChangeTime);

  stateHandling();

  switch (currentCase) {

    // -------------------------------------------------------------------------
    case 0: // Home
    // -------------------------------------------------------------------------
      if (!printed) {
        tft.fillScreen(ST77XX_BLACK);

        // Title
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        tft.setTextSize(2);
        tft.setCursor(35, 10);
        tft.print("Steps");
        tft.drawLine(30, 25, 95, 25, ST77XX_WHITE);

        // Pace label heading
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        tft.setTextSize(1);
        tft.setCursor(0, 120);
        tft.print("Pace");

        drawButtonLegend("Next", "Action", "Back");

        printed = true;
      }

      // Calibration warning
      if (!calibrated && !calibrationWarningDrawn) {
        tft.setTextSize(1);
        tft.setCursor(0, 70);
        tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
        tft.print("Not Calibrated,");
        tft.setCursor(0, 80);
        tft.print("Press Action to");
        tft.setCursor(0, 90);
        tft.print("Calibrate");
        calibrationWarningDrawn = true;
      }

      if (actionPressed) {
        currentCase = 2;
        subState    = 0;
        printed     = false;
        calibrationWarningDrawn = false;
        tft.fillScreen(ST77XX_BLACK);
      }

      {
        stepCounterInstance.runStepTrack();
        stepCounterInstance.maxMagnitude();

        stepsTaken  = stepCounterInstance.numberOfSteps();
        int rawPace = pacingIDInstance.paceTracker(stepsTaken);
        int pace    = lastPace;

        if (stepsTaken != lastValue || currentCase != previousCase) {
          tft.setCursor(55, 40);
          tft.setTextSize(3);
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.print(stepsTaken);
          lastValue = stepsTaken;
        }

        if (rawPace != -1) {
          pace = rawPace;
        }

        if (pace != lastPace || currentCase != previousCase) {
          drawPaceLabel(pace);
          lastPace = pace;
        }

        previousCase = currentCase;
      }
      break;


    // -------------------------------------------------------------------------
    case 1: // Exercise
    // -------------------------------------------------------------------------

      // --- subState 0: Prompt user ---
      if (subState == 0) {
        if (!printed) {
          tft.fillScreen(ST77XX_BLACK);
          drawScreenHeader("Exercise");
          drawButtonLegend("Next", "Start", "Back");
          printed = true;
        }

        if (actionPressed) {
          subState      = 1;
          printed       = false;
          stepsTaken    = stepCounterInstance.numberOfSteps();
          elapsedSeconds = 0;
          lastValue     = -1;
          lastPace      = -1;
          lastTimerUpdate = millis();
        }
      }

      // --- subState 1: Running exercise ---
      else if (subState == 1) {

        // Handle paused state first
        if (isPaused) {
          if (!pausedDrawn) {
            tft.setTextSize(1);
            tft.setCursor(45, 80);
            tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
            tft.print("Paused");
            pausedDrawn = true;
          }

          if (nextPressed) {
            // Resume
            tft.setCursor(45, 80);
            tft.setTextColor(ST77XX_BLACK, ST77XX_BLACK);
            tft.print("Paused");
            isPaused    = false;
            pausedDrawn = false;
            lastTimerUpdate = millis();
          }

          if (previousPressed) {
            // Stop → go to results
            subState    = 2;
            isPaused    = false;
            pausedDrawn = false;
            printed     = false;
            lastTimerUpdate = millis();
          }
        }
        else {
          if (actionPressed) {
            isPaused    = true;
            pausedDrawn = false;
          }

          if (previousPressed) {
            subState    = 2;
            isPaused    = false;
            pausedDrawn = false;
            printed     = false;
            break;
          }

          if (!printed) {
            tft.fillScreen(ST77XX_BLACK);

            // Timer label
            tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
            tft.setTextSize(2);
            tft.setCursor(5, 10);
            tft.print("Time: ");

            tft.drawLine(15, 30, 115, 30, ST77XX_WHITE);

            // Pace label heading
            tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
            tft.setTextSize(1);
            tft.setCursor(0, 120);
            tft.print("Pace");

            drawButtonLegend("Start", "Pause", "Stop");

            printed = true;
          }

          {
            stepCounterInstance.runStepTrack();
            stepCounterInstance.maxMagnitude();

            exerciseSteps = stepCounterInstance.numberOfSteps() - stepsTaken;
            int rawPace   = pacingIDInstance.paceTracker(exerciseSteps);
            int pace      = lastPace;
            distance      = stepCounterInstance.distanceTravelled(height);

            if (exerciseSteps != lastValue) {
              tft.setCursor(55, 40);
              tft.setTextSize(3);
              tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
              tft.print(exerciseSteps);
              lastValue = exerciseSteps;

              tft.setTextSize(1);
              tft.setCursor(55, 70);
              tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
              tft.print(distance, 1);
            }

            if (rawPace != -1) {
              if (rawPace == pendingPace) {
                paceConfirmCount++;
              } else {
                pendingPace      = rawPace;
                paceConfirmCount = 1;
              }
              if (paceConfirmCount >= PACE_CONFIRM_THRESHOLD) {
                pace             = pendingPace;
                paceConfirmCount = 0;
              }
            }

            if (pace != lastPace) {
              drawPaceLabel(pace);
              lastPace = pace;
            }

            // Update timer every second
            unsigned long now = millis();
            if (now - lastTimerUpdate >= 1000) {
              elapsedSeconds++;
              lastTimerUpdate = now;
              tft.setTextSize(2);
              tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
              tft.fillRect(10, 100, 80, 20, ST77XX_BLACK);
              tft.setCursor(65, 10);
              tft.print(elapsedSeconds);
              tft.print("s");
            }
          }

          previousCase = currentCase;
        }
      }

      // --- subState 2: Results ---
      else if (subState == 2) {
        if (!printed) {
          tft.fillScreen(ST77XX_BLACK);
          drawScreenHeader("Results");

          tft.setTextSize(1);
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);

          tft.setCursor(0, 40);
          tft.print("Steps: ");
          tft.setCursor(80, 40);
          tft.print(exerciseSteps);

          tft.setCursor(0, 70);
          tft.print("Time: ");
          tft.setCursor(70, 70);
          tft.print(elapsedSeconds);
          tft.print("s");

          tft.setCursor(0, 100);
          tft.print("Walked: ");
          tft.setCursor(80, 100);
          tft.print(distance, 1);
          tft.print("m");

          printed = true;
        }

        if (actionPressed) {
          calibrationInstance.reset();
          subState  = 0;
          printed   = false;
          distance  = 0.0f;
        }
      }
      break;


    // -------------------------------------------------------------------------
    case 2: // Calibration
    // -------------------------------------------------------------------------

      // --- subState 0: Prompt ---
      if (subState == 0) {
        if (!printed) {
          tft.fillScreen(ST77XX_BLACK);
          drawScreenHeader("Calibratio");
          tft.setTextSize(1);
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.setCursor(0, 28);
          tft.println("Please place tracker");
          tft.println("on a flat surface,");
          tft.println("then press action.");
          printed = true;
        }

        if (actionPressed) {
          subState = 1;
          printed  = false;
        }
      }

      // --- subState 1: Running calibration ---
      else if (subState == 1) {
        if (!printed) {
          calibrated = true;
          tft.fillScreen(ST77XX_BLACK);
          drawScreenHeader("Calibratio");
          tft.setTextSize(1);
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.setCursor(0, 30);
          tft.print("Calibrating...");
          printed = true;
        }

        bool done = calibrationInstance.calibrateAll();

        if (done) {
          tft.fillScreen(ST77XX_BLACK);
          subState         = 2;
          printed          = false;
          lastHeightPrinted = -9999.0f;
        }
      }

      // --- subState 2: Height adjustment ---
      else if (subState == 2) {
        if (nextPressed) {
          height  += 0.1f;
          printed  = false;
        } else if (previousPressed) {
          height  -= 0.1f;
          printed  = false;
        }

        if (!printed) {
          tft.setTextSize(2);
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.setCursor(0, 0);
          tft.print("Height");
          tft.setCursor(0, 20);
          tft.print("Adjustment");
          drawButtonLegend("Increase", "Accept", "Decrease");
          printed = true;
        }

        if (height != lastHeightPrinted) {
          tft.setTextSize(3);
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.fillRect(70, 50, 60, 24, ST77XX_BLACK);
          tft.setCursor(40, 70);
          tft.print(height, 1);
          lastHeightPrinted = height;
        }

        if (actionPressed) {
          stepCounterInstance.sensitivityAdjustment(height);
          subState = 3;
          printed  = false;
        }
      }

      // --- subState 3: Success ---
      else if (subState == 3) {
        if (!printed) {
          tft.fillScreen(ST77XX_BLACK);
          drawScreenHeader("Calibratio", ST77XX_GREEN);
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
          printed  = false;
        }
      }
      break;


    // -------------------------------------------------------------------------
    case 3: // Self Test
    // -------------------------------------------------------------------------

      // --- subState 0: Entry prompt ---
      if (subState == 0) {
        if (!printed) {
          tft.fillScreen(ST77XX_BLACK);
          drawScreenHeader("Self Test");
          tft.setTextSize(1);
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.setCursor(0, 28);
          tft.println("Press action to");
          tft.println("begin self test.");
          printed = true;
        }

        if (actionPressed) {
          subState = 1;
          printed  = false;
          selfTestInstance.resetSelfTest();
        }
      }

      // --- subState 1: Run axes sequentially ---
      else if (subState == 1) {
        if (!printed) {
          tft.fillScreen(ST77XX_BLACK);
          drawScreenHeader("Self Test");
          tft.setTextSize(1);
          tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
          tft.setCursor(0, 28);
          tft.println("Testing...");
          printed = true;
        }

        result = selfTestInstance.selfTestData(selfTestInstance.currentAxis);

        if (selfTestInstance.selfTestComplete()) {
          axisWorking[selfTestInstance.currentAxis] = result;
          selfTestInstance.resetSelfTest();
          selfTestInstance.currentAxis++;

          if (selfTestInstance.currentAxis >= 3) {
            selfTestInstance.currentAxis = 0;
            subState = 2;
            printed  = false;
          }
        }
      }

      // --- subState 2: Results ---
      else if (subState == 2) {
        if (!printed) {
          tft.fillScreen(ST77XX_BLACK);
          drawScreenHeader("Results");
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

        if (actionPressed) {
          subState = 0;
          printed  = false;
        }
      }
      break;
  }
}