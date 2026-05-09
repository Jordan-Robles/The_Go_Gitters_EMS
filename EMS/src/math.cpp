#include <Arduino.h>

float averageArray(float array[], int size){
  //Helper function to take the average of recorded accelermoter values 
  float sum = 0.0;
  for(int i = 0; i <size; i++){
    sum = sum + array[i];
  }
  float average = abs(sum / size);

  return average;
}