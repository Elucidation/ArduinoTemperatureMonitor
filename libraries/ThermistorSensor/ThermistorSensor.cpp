#include "ThermistorSensor.h"
#include "Arduino.h"

#define THERMISTORSENSOR_FILTER_ALPHA 3/10

// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 10000  


// Initialize sensor
ThermistorSensor::ThermistorSensor(int pin) {
  _pin = pin;
}

void ThermistorSensor::init() {
  // No setup needed.
}

// Wrapper on getting temperature data from analog sensor using steinhart
// conversion.
float ThermistorSensor::getReading() {
  uint8_t i;
  float average;
  float temperature;
 
  // Take N samples in a row, with a slight delay
  for (i=0; i< NUMSAMPLES; i++) {
   samples[i] = analogRead(_pin);
  }
 
  // Average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
     average += samples[i];
  }
  average /= NUMSAMPLES;

  // Convert the value to resistance
  average = 1023.0 / average - 1;
  average = SERIESRESISTOR / average;
  temperature = average / THERMISTORNOMINAL;          // (R/Ro)
  temperature = log(temperature);                     // ln(R/Ro)
  temperature /= BCOEFFICIENT;                        // 1/B * ln(R/Ro)
  temperature += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  temperature = 1.0 / temperature;                    // Invert
  temperature -= 273.15;                              // convert to C

  return temperature;
}

float ThermistorSensor::getFilteredReading() {
  float diff = getReading() - _last;
  _last += diff * THERMISTORSENSOR_FILTER_ALPHA;
  return _last;
}
