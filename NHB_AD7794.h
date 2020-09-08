/*
  NHB_AD7794.h - Library for using the AD7794 ADC
  Original created by Jaimy Juliano, December 28, 2010

  This file is part of the NHB_AD7794 library.

  MIT License

  Copyright (C) 2010,2019  Jaimy Juliano, NHBSystems

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

#ifndef NHB_AD7794_h
#define NHB_AD7794_h

#include <SPI.h>

#define CHANNEL_COUNT  8 //6 + temp and AVDD Monitor
//Communications register settings
#define WRITE_MODE_REG 0x08 //selects mode reg for writing
#define WRITE_CONF_REG 0x10 //selects conf reg for writing
#define READ_DATA_REG  0x58 //selects data reg for reading

#define DEFAULT_MODE_REG  0x2001     //Single conversion mode, Fadc = 470Hz
#define DEFAULT_CONF_REG  0x0010     //CH 0 - Bipolar, Gain = 1, Input buffer enabled
  
#define ADC_MAX_UP     16777216
#define ADC_MAX_BP     8388608
//#define READ_DELAY     10   //delay (in mS) to wait for conversion
#define INTERNAL_REF_V 1.17


struct channelSettings
{
  //Set some defaults
  uint8_t gain       = 1;
  bool isBuffered = true;
  bool isBipolar = true;
  bool isEnabled  = false;
  bool vBiasEnabled = false;
  uint8_t refMode = 0;
  float offset = 0.0;
};


class AD7794
{
  
  public:
    AD7794(uint8_t csPin, uint32_t spiFrequency, double refVoltage);
    void begin();
    void reset();

    void setBipolar(uint8_t ch, bool isBipolar);
    void setInputBuffer(uint8_t ch, bool isBuffered);
    void setGain(uint8_t ch, uint8_t gain);
    void setEnabled(uint8_t ch, bool enabled);
    void setVBias(uint8_t ch, bool enabled);
    void setRefMode(uint8_t ch, uint8_t mode);

    //void setUpdateRate(uint8_t bitMask);
    void setUpdateRate(double rate);
    void setConvMode(bool isSingle);

    uint32_t getReadingRaw(uint8_t ch);
    //float getReadingVolts(uint8_t ch);
    float TempSensorRawToDegC(uint32_t rawData);

    void read(float *buf, uint8_t bufSize); //experimental
    float read(uint8_t ch);
    void zero(uint8_t ch);  //Single channel
    void zero();            //All enabled, external channels (not internal temperature or VCC monitor)
    float offset(uint8_t ch);

  private:
    //Private helper functions
    void startConv();
    uint32_t getConvResult();
    void writeConfReg();
    void writeModeReg();
    void buildConfReg();
    void setActiveCh(uint8_t ch);
    uint8_t getGainBits(uint8_t gain);

    

    uint8_t CS;
    uint8_t currentCh;
    //unsigned long frequency;
    //uint8_t gain[3];
    float vRef;

    //ChannelClass Channel[CHANNEL_COUNT];
    channelSettings Channel[CHANNEL_COUNT];
    SPISettings spiSettings;

    uint16_t modeReg; //holds value for 16 bit mode register
    uint16_t confReg; //holds value for 16 bit configuration register

    bool isSnglConvMode;

    const uint16_t convTimeout = 480; // This should be set based on update rate eventually

};

#endif
