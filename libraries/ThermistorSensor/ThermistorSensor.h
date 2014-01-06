/*
  ThermistorSensor.h - Library for getting light level readings from an LED.
  Created by Sameer Ansari, Oct 10, 2013.
  Released into public domain.
*/

#ifndef ThermistorSensor_H
#define ThermistorSensor_H

// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 20

class ThermistorSensor
{
public:
  ThermistorSensor(int pin);
  float getReading();
  float getFilteredReading();
  void init();

private:
  int _pin;
  float _last;
  
  // Thermistor sample array
  int samples[NUMSAMPLES];
};

#endif
