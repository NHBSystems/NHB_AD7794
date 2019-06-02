/*
AD7794.h - Library for using the AD7794 ADC
Created by Jaimy Juliano, December 28, 2010

UPDATED 2-2018 include SPI transactions and tested on SAMD21 and Teensy 3.2
UPDATED 4-7-18 V02 - Lots of changes, WIP
*/

#ifndef Ad7794V_02_h
#define Ad7794V_02_h

#include <SPI.h>

#define CHANNEL_COUNT  8 //6 + temp and AVDD Monitor
//Communications register settings
#define WRITE_MODE_REG 0x08 //selects mode reg for writing
#define WRITE_CONF_REG 0x10 //selects conf reg for writing
#define READ_DATA_REG  0x58 //selects data reg for reading
#define ADC_MAX_UP     16777216
#define ADC_MAX_BP     8388608
//#define READ_DELAY     10   //delay (in mS) to wait for conversion


struct channelSettings
{
  //Set some defaults
  uint8_t gain       = 1;
  bool isBuffered = true;
  bool isUnipolar = false;
  bool isEnabled  = false;
  float offset = 0.0;
};

// class ChannelClass
// {
//   public:
//     uint8_t gain       = 1;
//     bool isBuffered = true;
//     bool isUnipolar = false;
//     bool isEnabled  = true;
//
//     setBipolar(bool isBipolar);
//     setGain(uint8_t gain);
//   protected:
// }

class AD7794
{
  // friend class ChannelClass;

  public:
    AD7794(uint8_t csPin, uint32_t spiFrequency, double refVoltage);
    void begin();
    void reset();

    void setBipolar(uint8_t ch, bool isBipolar);
    void setInputBuffer(uint8_t ch, bool enabled);
    void setGain(uint8_t ch, uint8_t gain);
    void setEnabled(uint8_t ch, bool enabled);

    //void setUpdateRate(uint8_t bitMask);
    void setUpdateRate(double rate);
    void setConvMode(bool isSingle);

    uint32_t getReadingRaw(uint8_t ch);
    //float getReadingVolts(uint8_t ch);

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
    void buildConfReg(uint8_t ch);
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
