
//Setting input pins for the ADXL to the Arduino nano
const int x = A0;
const int y = A1;
const int z = A2;
const int stPin = 13;

//Setting input pins for the buttons
const int button1 = 2;
const int button2 = 3;
const int button3 = 4;

//Setting pins for LED
const int led1 = 5;
const int led2 = 6;
const int led3 = 7;

//Setting pins for the GC9A01
const int DIN = A4;
const int CLK = A5;
const int CS = 10;
const int DC = 9;



//Set input pins for the ADXL to the Arduino nano
int case = 0; // Set case = 0 to initialise system to starting profile
int selfTestCount[] = [0, 0, 0]; // used to keep track of seft test pins
void setup() {
  // Set pinMode for x, y, z pins as INPUT
  pinMode(x, INPUT);
  pinMode(y,INPUT);
  pinMode(z,INPUT);
  pinMode(stPin,OUTPUT);

  serial.Begin(9600);

}
void loop() {
  switch(case){
    case 0:

    if //Self test button pressed == HIGH
      case = 1; // Test X
      //Print to Screen
    else // Continue with step tracking routine 
    break;

    case 1:
     //anologRead(x)
     if // x in == expected x value
      //print to screen "X axis Good"
      selfTestCount[0] = 1;
      case = 2; // Test Y
      else if //x in != expected x value
        //print to screen "X axis no good"
        case = 2; // Test Y
    break;

    case 2:
      //anologRead(x)
     if // y in == expected y value
      //print to screen "Y axis Good"
      selfTestCount[1] = 1;
      case = 3; // Test Z
      else if //y in != expected y value
        //print to screen "y axis no good"
    break;

    case 3:
      //anologRead(x)
     if // z in == expected z value
      //print to screen "Z axis Good"
      selfTestCount[2] = 1;
      case = 1; // Go back to Case 0 to resume step tracking routine
      else if //z in != expected zvalue
        //print to screen "Z axis no good"
    break;
  }
}










