# NHB_AD7794
Arduino Library for the Analog Devices AD7794 24bit ADC

This is essentially just a new fork of the [JJ_AD7794](https://github.com/jjuliano77/JJ_AD7794) library. I have also changed to an MIT license.

This library has been tested with ATMEGA328, ATMEGA32u4, SAMD21, Teensy 3.2, and Esp8266* (The Esp8266 has goofy SPI and requires a some workarounds).  
  
Basic API  
--------


### Constructor
```c
AD7794(uint8_t csPin, uint32_t spiFrequency, double refVoltage);
```
|Arg|Description|
| ------ | --------------- |
|*csPin* | Chip Select pin |
|*spiFrequency*|SPI bus frequency to use|
|*refVoltage*  |Reference voltage. Currently all NHBSystems boards use a 2.5V       reference, however if you are using the raw chip, you should set this value to whatever you are using. If set to **1.17** or **AD7794_INTERNAL_REF_V** the internal 1.17 volt reference will be selected.|

--------------------------

### Filter Update Rate
The update rate is set for all channels. This setting determines the output data rate and filtering (noise rejection).
```c
    void setUpdateRate(double rate);
```
|Arg|Description|
| ------ | --------------- |
|*rate* | Update rate in Hz. There are only 15 valid settings, but any value entered will be mapped to the next closest valid option. The following table shows a subset of valid settings. Refer to the data sheet for the full list, however these are the only ones I ever use. 

|Freq (Hz)| Settling time (ms) | Noise Rejection |
| ------- | ------------------ | --------------- |
|  470    | 4                  | ---             |
|  19.6   | 101                | 90 dB (60 Hz only)     |
|  16.7   | 120                | 65 dB (50 Hz and 60 Hz)|
|  10     | 200                | 69 dB (50 Hz and 60 Hz)|

-------------------------------

### Channel Settings
The basic methods for setting up the individual channels are:
```c
    void setBipolar(uint8_t ch, bool isBipolar);
    void setGain(uint8_t ch, uint8_t gain);
    void setEnabled(uint8_t ch, bool enabled);
```
They each share the basic signature of `function(Channel, setting)` so I have grouped the discriptions below.

|Arg|Description|
| ------ | --------------- |
|*ch* | The AIN channel to act on |
|*isBipolar*| Sets whether the channel is configured as bipolar or unipolar. Note: Bipolar is centered arround the middle of the of the input range. It does not mean you can read voltages below VSS|
|*gain*  |Sets the gain for the given channel. Acceptable values are 1, 2, 4, 8, 16, 32, 64, 128 |
|*enabled*| enable/disable the channel|

-------------------------

### Getting Readings
There are a few different methods to get readings from the ADC.
```c
uint32_t getReadingRaw(uint8_t ch);
```
`getReadingRaw(chan)` Returns the raw ADC counts for the given channel 

```c 
float read(uint8_t ch);
```
`read(chan)` Returns a reading for the given channel
```c

void read(float *buf, uint8_t bufSize);
```
`read(buffer,size)` Is just for convenience and can be used to read a nuber of channels at once, though they must start at 0 and be sequential. (e.g. 0 trough 3, or 0 trough 5). 

#### Reading Temperature
Also, the onboard temperature sensor can be read by reading channel 6. Note, it may be off by a couple of degrees and need an offset correction applied. This is shown in the thermocouple example sketch.
```c
float tempC = read(6); //Returns temperature in Celsius, may need offset corection
```

------------------------

Example
-----------
```c
#include "NHB_AD7794.h"

//Pin defines, change for your setup
#define AD7794_CS  10  
#define EX_EN_PIN  9  

AD7794 adc(AD7794_CS, 1000000, 2.50);
float readings[6];

void setup() {
  
    Serial.begin(115200);

    while(!Serial); // Wait for serial on native USB boards
    
    // NHB boards use a pin to control excitation voltage, set 
    // it to output, and write LOW to turn on the excitation
    pinMode(EX_EN_PIN, OUTPUT); 
    digitalWrite(EX_EN_PIN,LOW);  //low  = 2.5 Vex ON

    adc.begin();
  
    adc.setUpdateRate(470); //Fastest setting, no filtering

    for(int i=0; i < 6; i++){
        adc.setBipolar(i,true);    
        adc.setGain(i, 128);    
        adc.setEnabled(i,true);
    }    
}

void loop() {

    //Read all 6 AIN channels
    adc.read(readings,6);  

    //Dump voltage readings out to serial port.
    for(int i=0; i < 6; i++){
        Serial.print(readings[i],DEC);
        Serial.print('\t');
    }  
  
    Serial.println();

    delay(10); //wait a bit
}
```