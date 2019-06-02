/*
JJ_AD7794.cpp - Library for using the AD7794 ADC
Original created by Jaimy Juliano, December 28, 2010

Copyright (C) 2010,2019  Jaimy Juliano

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
along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "JJ_AD7794.h"
#include <SPI.h>


AD7794::AD7794(uint8_t csPin, uint32_t spiFrequency, double refVoltage = 2.50)
{
  //pinMode(csPin, OUTPUT);
  CS = csPin;

  #if defined (ESP8266)
    //For some reason we need to use MODE2 on ESP8266, whitch makes no sense at all
    //In fact, neither MODE3 or MODE2 are supposed to be supported!
    spiSettings = SPISettings(spiFrequency,MSBFIRST,SPI_MODE2); 
  #else
    //Should be MODE3
    spiSettings = SPISettings(spiFrequency,MSBFIRST,SPI_MODE3); 
  #endif
  
  vRef = refVoltage;

  //Default register settings
  modeReg = 0x2001;     //Single conversion mode, Fadc = 470Hz
  confReg = 0x0010;     //CH 0 - Bipolar, Gain = 1, Input buffer enabled

  isSnglConvMode = true;
  
}

void AD7794::begin()
{
  SPI.begin();

  reset();
  delay(2); //4 times the recomended period

  //Apply the defaults that were set up in the constructor
  //Should add a begin(,,) method that lets you override the defaults
  for(uint8_t i = 0; i < CHANNEL_COUNT-2; i++){ //<-- Channel count stuff needs to be handled better!!
    setActiveCh(i);
    writeModeReg();
    writeConfReg();
  }

  setActiveCh(0); //Set channel back to 0

  //TODO: add any other init code here
}
////////////////////////////////////////////////////////////////////////
//Write 32 1's to reset the chip
void AD7794::reset()
{
  // Speed set to 4MHz, SPI mode set to MODE 3 and Bit order set to MSB first.
  SPI.beginTransaction(spiSettings);
  digitalWrite(CS, LOW); //Assert CS
  for(uint8_t i=0;i<4;i++)
  {
    SPI.transfer(0xFF);
  }
  digitalWrite(CS, HIGH);
  SPI.endTransaction();
}

//Sets bipolar/unipolar mode for currently selected channel
void AD7794::setBipolar(uint8_t ch, bool isBipolar)
{
  setActiveCh(ch);
  Channel[currentCh].isUnipolar = false;
  buildConfReg(currentCh);
  writeConfReg();
}

void AD7794::setInputBuffer(uint8_t ch, bool enabled)
{
  //TODO: Something ;)
}

void AD7794::setGain(uint8_t ch, uint8_t gain)
{
  setActiveCh(ch);
  Channel[currentCh].gain = gain;
  buildConfReg(currentCh);
  writeConfReg();
}

void AD7794::setEnabled(uint8_t ch, bool enabled)
{
  Channel[currentCh].isEnabled = enabled;
}

/******************************************************
Sets low byte of the mode register. The high nibble
must be 0, the low nibble sets the mode (FS3->FS0)
[EXAMPLE '\x01' = 470Hz]
refer to datasheet for modes
  FS3 | FS2 | FS1 | FS0 | frequency (hz) | Tsettle (ms)
  0   |  0  |  0  |  0  |  X             |  X
  0   |  0  |  0  |  1  |  470           |  4
  0   |  0  |  1  |  0  |  242           |  8
  0   |  0  |  1  |  1  |  123           |  16
  0   |  1  |  0  |  0  |   62           |  32
  ....
*/
void AD7794::setUpdateRate(double rate)
{
  uint8_t bitMask;
  //Map requested to next available range
  if(rate <= 4.17)
    {bitMask = 0x0F;}
  else if(rate > 4.17 && rate <= 6.25)
    {bitMask = 0x0E;}
  else if(rate > 6.25 && rate <= 8.33)
    {bitMask = 0x0D;}
  else if(rate > 8.33 && rate <= 10)
    {bitMask = 0x0C;}
  else if(rate > 10 && rate <= 12.5)
    {bitMask = 0x0B;}
  else if(rate > 12.5 && rate <= 16.7)
    {bitMask = 0x0A;}     //This ones wierd 0x0A = 80 dB(50Hz)
    //{bitMask = 0x09;}   //or 0x09 = 65 dB(50Hx and 60Hz)
  else if(rate > 16.7 && rate <= 19.6)
    {bitMask = 0x08;}
  else if(rate > 19.6 && rate <= 33.2)
    {bitMask = 0x07;}
  else if(rate > 33.2 && rate <= 39)
    {bitMask = 0x06;}
  else if(rate > 39 && rate <= 50)
    {bitMask = 0x05;}
  else if(rate > 50 && rate <= 62)
    {bitMask = 0x04;}
  else if(rate > 62 && rate <= 123)
    {bitMask = 0x03;}
  else if(rate > 123 && rate <= 242)
    {bitMask = 0x02;}
    else if(rate > 242)
      {bitMask = 0x01;} //anything above 242 set to 470 Hz

  modeReg &= 0xFF00; //Zero off low byte
  modeReg |= bitMask;

  writeModeReg();
  //TODO: Put table from datasheet in comments, in header file
}

void AD7794::setConvMode(bool isSingle)
{
  if(isSingle == true){
    isSnglConvMode = true;
    modeReg &= 0x00FF;
    modeReg |= 0x2000;
  }
  else{
    isSnglConvMode = false;
    modeReg &= 0x00FF;
  }

  writeModeReg();
}

// OK, for now I'm not going to handle the spi transaction inside the startConversion()
// and getConvResult() functions. They are very low level and the behavior is different
// depending on conversion mode (and other things ?). think I will make these private
// and create a higher level functions to get readings.
uint32_t AD7794::getConvResult()
{
  uint8_t inByte;
  uint32_t result = 0;

  //SPI.beginTransaction(spiSettings); //We should still be in our transaction
  SPI.transfer(READ_DATA_REG);

  //Read 24 bits one byte at a time, and put in an unsigned long
  inByte = SPI.transfer(0xFF); //dummy byte
  result = inByte;
  inByte = SPI.transfer(0xFF); //dummy byte
  result = result << 8;
  result = result | inByte;
  inByte = SPI.transfer(0xFF); //dummy byte
  result = result << 8;
  result = result | inByte;

  //De-assert CS if not in continous conversion mode
  if(isSnglConvMode == true){
    digitalWrite(CS,HIGH);
  }

  //SPI.endTransaction(); //Need to look into how to handle continous conversion mode
  return result;
}

//Experiment with reading all active channels, this may be the way I go in the future UNTESTED
void AD7794::read(float *buf, uint8_t bufSize)
{
  uint8_t readingCnt = 0 ;

  for(int i = 0;  i < CHANNEL_COUNT; i++){
    if(Channel[i].isEnabled){
      if(readingCnt < bufSize){
        buf[readingCnt] = read(i);
      }else{
        //buffer too small!
        //Maybe return an error code?
      }
      readingCnt++;
    }
  }
}

/* read - The single channel version
   Gets reading from a single channel. Right now it
   is in volts (default scailing), but I may have it use
   a scaling setting stored for each channel. I might also
   just save that for a higher level hardware abstraction class.
*/
float AD7794::read(uint8_t ch)
{
  //Lets the conversion result
  uint32_t adcRaw = getReadingRaw(ch);
  //Serial.print(adcRaw);
  //Serial.print(' ');
  float result;

  if(ch == 6){ //Channel 6 is temperature, handle it differently 1.17 V internal Ref
    return (((float)adcRaw / ADC_MAX_BP - 1) * 1.17); //Bipolar, not sure what mode for temp sensor
  }

  //And convert to Volts, note: no error checking
  if(Channel[currentCh].isUnipolar){
    result = (adcRaw * vRef) / (ADC_MAX_UP * Channel[currentCh].gain);            //Unipolar formula
    //Serial.print("unipolar");
  }
  else{
    result = (((float)adcRaw / ADC_MAX_BP - 1) * vRef) / Channel[currentCh].gain; //Bipolar formula    
    //Serial.print("bipolar");
  }

  return result - Channel[currentCh].offset;
}

/* zero - Record offsets to apply to the channel (if active) */
void AD7794::zero(uint8_t ch)
{
  if(Channel[ch].isEnabled == true){
    Channel[ch].offset = read(ch);
    //Channel[ch].offset = adc.getReadingRaw(ch); //Have to test this, not sure if it will work as expected
  }                                               //for both unipolar and bipolar modes
}

/* zero - Record offsets to apply to all active internal
   channels. (not temperature an AVDD monitor)
*/
void AD7794::zero()
{
  for(int i = 0;  i < CHANNEL_COUNT-2 ; i++){
    zero(i);
  }
}

/* offset - Returns channel offset value. I'm not sure if this will be needed
   in the long run, but it's usefull for testing now.
*/
float AD7794::offset(uint8_t ch)
{
  return Channel[ch].offset;
}


// This function is BLOCKING. I'm not sure if it is even possible to make a
// non blocking version with this chip on a shared SPI bus.
uint32_t AD7794::getReadingRaw(uint8_t ch)
{
  SPI.beginTransaction(spiSettings);

  setActiveCh(ch);
  startConv();

  //Attempt to replace delay with polling the DOUT/RDY (MISO) line
  //Seems to work, TODO: figure out how to make this work on
  //non-standard SPI pins
  uint32_t t = millis();

  #if defined (ESP8266) //Workaround until I figure out how to read the MISO pin status on the ESP8266
    delay(6);           //This delay is only appropriate for the fastest rate (470 Hz)
  #else
    while(digitalRead(MISO) == HIGH){
    //while(digitalRead(12) == HIGH){ //ESP8266 troubleshooting
      if((millis() - t) >= convTimeout){        
        //Serial.print("getReadingRaw Timeout");
        break;
      }  
    }
  #endif

  uint32_t adcRaw = getConvResult();

  SPI.endTransaction();
  return adcRaw;
}

void AD7794::setActiveCh(uint8_t ch)
{
  if(ch < CHANNEL_COUNT){
    currentCh = ch;
    buildConfReg(currentCh);
    writeConfReg();
  }
}

// OK, for now I'm not going to handle the spi transaction inside the startConversion()
// and getConvResult() functions. They are very low level and the behavior is different
// depending on conversion mode (and other things ?). think I will make these private
// and create a higher level functions to get readings.
void AD7794::startConv()
{
  //Write out the mode reg, but leave CS asserted (LOW)
  //SPI.beginTransaction(spiSettings);
  digitalWrite(CS,LOW);
  SPI.transfer(WRITE_MODE_REG);
  SPI.transfer(highByte(modeReg));
  SPI.transfer(lowByte(modeReg));
  //Don't end the transaction yet. for now this is going to have to hold on
  //to the buss until the conversion is complete. not sure if there is a way around this
}


//////// Private helper functions/////////////////
void AD7794::buildConfReg(uint8_t ch)
{
  //Could the use of bitWrite be the problem with the ESP8266? -JJ 1-23-2019 (UPDATE: No doesn't seem to matter)
  //prints added for troubleshooting
  // Serial.print(confReg,BIN);
  // Serial.print(' ');

  confReg = (getGainBits(Channel[currentCh].gain) << 8) | ch;

  // Serial.print(confReg,BIN);
  // Serial.print(' ');

  bitWrite(confReg,12,Channel[currentCh].isUnipolar);
  bitWrite(confReg,4,Channel[currentCh].isBuffered);

  // Serial.int(confReg,BIN);
  // Serial.int(' ');

}

void AD7794::writeConfReg()
{
  SPI.beginTransaction(spiSettings);
  digitalWrite(CS,LOW);
  SPI.transfer(WRITE_CONF_REG);
  
  //SPI.transfer(highByte(confReg));
  //SPI.transfer(lowByte(confReg));

  //Just for debugging esp8266
  //delay(1);
  SPI.transfer16(confReg);


  digitalWrite(CS,HIGH);
  SPI.endTransaction();
}

void AD7794::writeModeReg()
{
  SPI.beginTransaction(spiSettings);
  digitalWrite(CS,LOW);
  SPI.transfer(WRITE_MODE_REG);
  
  //SPI.transfer(highByte(modeReg));
  //SPI.transfer(lowByte(modeReg));

  //Just for debugging esp8266
  //delay(1);
  SPI.transfer16(modeReg);

  digitalWrite(CS,HIGH);
  SPI.endTransaction();
}

byte AD7794::getGainBits(uint8_t gain)
{
  uint8_t gainBits;

  switch(gain){
    case 1:
      gainBits |= 0x00;
      break;
    case 2:
      gainBits |= 0x01;
      break;
    case 4:
      gainBits |= 0x02;
      break;
    case 8:
      gainBits |= 0x03;
      break;
    case 16:
      gainBits |= 0x04;
      break;
    case 32:
      gainBits |= 0x05;
      break;
    case 64:
      gainBits |= 0x06;
      break;
    case 128:
      gainBits |= 0x07;
      break;
    default:
      //OOPS!, shouldn't be here, well just set it to 0x00 (gain = 1)
      gainBits |= 0x00;
      break;
  }
  return gainBits;
}
