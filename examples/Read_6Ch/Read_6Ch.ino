/*
  6 Channel Full bridge feather board example

  Reads all 6 channels and prints out on Serial. The readings are done
  in bipolar mode with a gain of 128. This would be appropriate for
  most full bridge type sensors like load cells and pressure gauges

  This file is part of the NHB_AD7794 library.

  MIT License

  Copyright (C) 2010,2019  Jaimy Juliano

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include <SPI.h>
#include "NHB_AD7794.h"


//You need to set the pins for your Feather here
//EX_EN_PIN only matters if you have configured the
//EX_EN jumper to controll excitation from your board

//Teensy 3.2 on Feather adapter
// #define AD7794_CS    3 
// #define EX_EN_PIN    9

//Feather M0 Basic Proto
#define AD7794_CS  10  
#define EX_EN_PIN  9  

// Huzzah ESP8266
// #define AD7794_CS   15 
// #define EX_EN_PIN    0


AD7794 adc(AD7794_CS, 4000000, 2.50);

float readings[8]; //6 channels + temp and AVDD monitor

void setup() {
  
   Serial.begin(115200);
   

  while(!Serial);
 
  // Uncomment next 2 lines if Jumper configured for EX control
  //pinMode(EX_EN_PIN, OUTPUT);
  //digitalWrite(EX_EN_PIN,LOW);  //low  = 2.5 Vex ON

  adc.begin();
  
  delay(100);
  
  adc.setUpdateRate(470);


  // Disabling "chop" reduces the settling time by a factor of 2, which will 
  // increase the effective sampling rate (the ADC conversion rate is 
  // the same, but the setting time is half). However, there may be more drift in 
  // the readings, and the maximum comon mode voltage is slightly reduced. See the 
  // datasheet for details. Default is enabled. Uncomment line below to disable
  //adc.setChopEnabled(false); 



  for(int i=0; i < 6; i++){
    adc.setBipolar(i,true);
    //delay(2);
    adc.setGain(i, 128);
    //delay(2);
    adc.setEnabled(i,true);
    //delay(2);
  }
   

  delay(100);  
  
  //Uncomment to zero out all channels    
  //adc.zero(); //Zero out all channels

}

void loop() {

  unsigned long dt;  
  unsigned long startTime = millis();

  
  adc.read(readings,6);  
  
  dt = millis() - startTime;

  Serial.print(voltsToEngUnits(readings[0], 5),DEC);
  Serial.print('\t');  
  Serial.print(voltsToEngUnits(readings[1], 5),DEC);
  Serial.print('\t');
  Serial.print(voltsToEngUnits(readings[2], 5),DEC);
  Serial.print('\t');
  Serial.print(voltsToEngUnits(readings[3], 5),DEC);
  Serial.print('\t');
  Serial.print(voltsToEngUnits(readings[4], 5),DEC);
  Serial.print('\t');
  Serial.print(voltsToEngUnits(readings[5], 5),DEC);
  
  Serial.print('\t');
  Serial.print(dt, DEC);
  Serial.println();

  delay(10); //wait some more
}


float voltsToEngUnits(float volts,float scaleFactor){
  const float vEx = 2.5;

  float mVpV = (volts * 1000) / vEx;
  return mVpV * scaleFactor;
}
