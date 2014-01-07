/*
  ThermistorSensor.h - Library for getting temperature readings from a 
  thermistor.
  Created by Sameer Ansari, Jan 1, 2014.
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
