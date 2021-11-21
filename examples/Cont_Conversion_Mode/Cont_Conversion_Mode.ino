/*
  Reads 1 channel in continuous mode at the maximum rate of 470 sps

  IMPORTANT! Continuous mode can only be used on one channel at a time, 
  and it will not allow sharing of the SPI bus while active. Do not use 
  this on any device that shares the same bus with other critical hardware.
  Because of this, continuous mode is not very practical for many applications.  

  The readings are taken in bipolar mode with a gain of 128. This would be 
  appropriate for most full bridge type sensors like load cells and pressure gauges

  This file is part of the NHB_AD7794 library.

  MIT License

  Copyright (C) 2021  Jaimy Juliano

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

#include <Arduino.h>
#include "NHB_AD7794.h"

#define AD7794_CS 10
#define EX_EN_PIN 9

AD7794 adc(AD7794_CS, 4000000, 2.50);

void setup()
{
  pinMode(EX_EN_PIN, OUTPUT);
  digitalWrite(EX_EN_PIN, LOW);

  adc.begin();

  adc.setUpdateRate(470);   

  adc.setBipolar(0, true);
  adc.setGain(0, 128);
  adc.setEnabled(0, true);

  adc.setMode(AD7794_OpMode_Continuous);
  
}

void loop()
{     
    double reading;     

    uint32_t count = 0;
    static uint32_t lastCount = 0;
    uint32_t stop = millis() + 1000;

    while (millis() < stop)
    {
        reading = adc.read(0);

        Serial.print(voltsToEngUnits(reading, 5),DEC);
        Serial.print('\t');
        Serial.print(lastCount);
        Serial.println("Hz");

        ++count;
    }

    lastCount = count;

}

float voltsToEngUnits(float volts,float scaleFactor){
  const float vEx = 2.5;

  float mVpV = (volts * 1000) / vEx;
  return mVpV * scaleFactor;
}