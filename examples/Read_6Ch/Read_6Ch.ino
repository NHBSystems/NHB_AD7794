/*
  6 Channel Full bridge feather board example

  Reads all 6 channels and prints out on Serial. The readings are done
  in bipolar mode with a gain of 128. This would be appropriate for
  most full bridge type sensors like load cells and pressure gauges

  Copyright (C) 2019  Jaimy Juliano

  This file is part of the JJ_AD7794 library.

  This library is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include <SPI.h>
#include "JJ_AD7794.h"

#if defined (ESP8266)
  #define AD7794_CS   16 //<- Should try this on 15 again
  #define EX_EN_PIN    0
#else
  //Teensy 3.2 on Feather adapter
  #define AD7794_CS    3 
  #define EX_EN_PIN    9

  //Some Other board
  //#define AD7794_CS  _  
  //#define EX_EN_PIN  _  
  
#endif

// Feather Huzzah ESP8266
// Note: It looks like the AD7794 only works with the ESP8266 when
// SPI Mode is set to MODE_2. This doesn't make much sense because
// Neither Mode_2 nor Mode_3 are supposed to be supported. Also, for
// some reason I cant read the MISO pin to see when a conversion is
// complete. For now I put a 6 mS delay in the library when it is
// compiled for the ESP8266 target.

AD7794 adc(AD7794_CS, 1000000, 2.50);

float readings[8], offsets[8]; //6 channels + temp and AVDD monitor

void setup() {
  // put your setup code here, to run once:
   Serial.begin(115200);
   //adc.begin(); //Added while troubleshooting - 5-6-18

   while(!Serial);
 
  pinMode(AD7794_CS, OUTPUT); //Need to do this
  pinMode(EX_EN_PIN, OUTPUT);

  digitalWrite(EX_EN_PIN,LOW);  //low  = 2.5 Vex ON

  adc.begin();
  
  delay(100);
  
  adc.setUpdateRate(470);

  for(int i=0; i < 6; i++){
    adc.setBipolar(i,true);
    //delay(2);
    adc.setGain(i, 128);
    //delay(2);
    adc.setEnabled(i,true);
    //delay(2);
  }
  

  delay(100);  
  
  //get readings and auto zero on startup
  //It seems that I have to take a throw away reading first to get a
  //decent offset value. I need to figure out why that is. -JJ 05-17-18
  
  double junk = adc.read(0);
  delay(10);
  
  adc.zero(); //Zero out all channels

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
