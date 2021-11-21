/*
  6 Channel Full bridge feather board example

  This example reads a K-Type thermocouple on channel 0 and does
  does cold junction compensation using the integrated temperature
  sensor in the AD7794.

  2 x 1 MOhm bias resistors are needed for this example to work correctly.
  1 between the GND (or VEX-) terminal and the TC- terminal and 1 between VCC
  (or VEX+) terminal and the TC+ terminal. (See diagram in the extras folder)

  UPDATE: The boards now include bias resistors on the bottom side that can
  be enabled with solder jumpers.

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
//#define AD7794_CS    3 
//#define EX_EN_PIN    9

//Pins for Feather M0 Basic Proto
#define AD7794_CS  10  
#define EX_EN_PIN  9    


#define TC_ADC_CHANNEL      0
#define IC_TEMP_ADC_CHANNEL 6


AD7794 adc(AD7794_CS, 4000000, 2.50);


//Offset correction for the IC internal temp sensor
//this will may need to be adjusted. Some chips seem to have
//some offset in the measurment from the on chip sensor 
const float icTempOffset = 0.0; 


//For moving avg filter below
float EMA_a = 0.1;
float tcEMA = 0.0;



void setup() {
  
  Serial.begin(115200);  

  while(!Serial);
   
  //Uncomment next 2 lines if Jumper configured for EX control
  //pinMode(EX_EN_PIN, OUTPUT);
  //digitalWrite(EX_EN_PIN,LOW);  //low  = 2.5 Vex ON

  adc.begin();
  
  delay(100);
  
  adc.setUpdateRate(19.6);
  

  adc.setBipolar(TC_ADC_CHANNEL,true);  
  adc.setGain(TC_ADC_CHANNEL,32);
  adc.setEnabled(TC_ADC_CHANNEL,true);
    
}

void loop() {


  // First read the internal IC temp on ch 6. The read method 
  // will convert to Deg C internally but here is how it is done:
  //
  // uint32_t icTempRaw = adc.getReadingRaw(IC_TEMP_ADC_CHANNEL);
  // float volts = ((float)icTempRaw - 0x800000) * (INTERNAL_REF_V / ADC_MAX_BP);
  // float degK = volts/0.00081; //Sensitivity = 0.81 mv/DegC
  // float degC = degK - 273;

  float icTemp = adc.read(IC_TEMP_ADC_CHANNEL) + icTempOffset;

  float referenceVoltage = Thermocouple_Ktype_TempToVoltageDegC(icTemp);

  
  float tc = adc.read(TC_ADC_CHANNEL);
  

  float compensatedVoltage = tc + referenceVoltage;
  float compensatedTemperature = Thermocouple_Ktype_VoltageToTempDegC(compensatedVoltage);

  //Try a simple exponential moving average filter
  //Formula from: //https://www.norwegiancreations.com/2015/10/tutorial-potentiometers-with-arduino-and-filtering/
  tcEMA = ( EMA_a * compensatedTemperature ) + ( ( 1 - EMA_a ) * tcEMA );
  
  

  Serial.print(icTemp,DEC);
  Serial.print('\t'); 

  Serial.print(compensatedTemperature,DEC);
  Serial.print('\t');

  Serial.print(tcEMA,DEC);
  Serial.print('\t');
  
  Serial.println();

  delay(100); //wait a bit
}

/***************************************************************************************************************
  The following K-type thermocouple functions were borrowed from here:
  https://github.com/annem/AD7193/blob/master/examples/AD7193_Thermocouple_Example/Thermocouple_Functions.ino

****************************************************************************************************************/

float Thermocouple_Ktype_VoltageToTempDegC(float voltage) {
   // http://srdata.nist.gov/its90/type_k/kcoefficients_inverse.html
   float coef_1[] = {0, 2.5173462e1, -1.1662878, -1.0833638, -8.9773540e-1};            // coefficients (in mV) for -200 to 0C, -5.891mv to 0mv
   float coef_2[] = {0, 2.508355e1, 7.860106e-2, -2.503131e-1, 8.315270e-2};            // coefficients (in mV) for 0 to 500C, 0mv to 20.644mv
   float coef_3[] = {-1.318058e2, 4.830222e1, -1.646031, 5.464731e-2, -9.650715e-4};    // whoa, that's hot...
   int i = 5;  // number of coefficients in array
   float temperature;

   float mVoltage = voltage * 1e3;

   if(voltage < 0) {
    temperature = power_series(i, mVoltage, coef_1);
   }else if (voltage > 20.644){
    temperature = power_series(i, mVoltage, coef_3);
   }else{
    temperature = power_series(i, mVoltage, coef_2);
   }

   return(temperature);
}

float Thermocouple_Ktype_TempToVoltageDegC(float temperature) {
  // https://srdata.nist.gov/its90/type_k/kcoefficients.html
  float coef_1[] = {0, 0.3945013e-1, 0.2362237e-4, -0.3285891e-6, -0.4990483e-8};               // coefficients (in mV) for -270 to 0C, -5.891mv to 0mv
  float coef_2[] = {-0.17600414e-1, 0.38921205e-1, 0.1855877e-4, -0.9945759e-7, 0.31840946e-9}; // coefficients (in mV) for 0 to 1372C, 0mv to ....
  float a_coef[] = {0.1185976, -0.1183432e-3, 0.1269686e3};
  int i = 5;  // number of coefficients in array

  float mVoltage;
  float a_power = a_coef[1] * pow((temperature - a_coef[2]), 2);
  float a_results = a_coef[0] * exp(a_power);

  if(temperature < 0) {
    mVoltage = power_series(i, temperature, coef_2) + a_results;
  } else {
    mVoltage = power_series(i, temperature, coef_1);
  }

  return(mVoltage / 1e3);
}

float power_series(int n, float input, float coef[])
 {
      //delay(10);      
      int i;
      float sum=coef[0];
      for(i=1;i<=(n-1);i++)
           sum=sum+(pow(input, (float)i)*coef[i]);
      return(sum);
 }

