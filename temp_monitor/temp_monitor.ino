// which analog pins to connect
#define THERMISTOR_PIN_A A0
#define THERMISTOR_PIN_B A1
// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 20
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 10000    

// PIN 2 - Used for Pushbutton with interrupt 0

// Thermistor sample array
int samples[NUMSAMPLES];

// Used in getTemp to get steinhart value for temperature conversion from thermistor reading.
float steinhart;

float tempA, tempB;

// Returns temperature in degrees celcius of thermistor reading from given pin
float getTemp(int pin) {
  uint8_t i;
  float average;
 
  // Take N samples in a row, with a slight delay
  for (i=0; i< NUMSAMPLES; i++) {
   samples[i] = analogRead(pin);
  }
 
  // Average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
     average += samples[i];
  }
  average /= NUMSAMPLES;

  // Convert the value to resistance
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C
  return steinhart;
}

bool last, do_switch = false;
void doFileSwitch() {
  // Todo: Switch to new logging file on button pres
  do_switch = true;
}

////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  
  // Trigger backlight on push button on pin 2 (interrupt 0)
  attachInterrupt(0, doFileSwitch, RISING);
  
  // Start serial
  Serial.begin(9600);
  
}

void loop() {
  // Read thermistors
  tempA = getTemp(THERMISTOR_PIN_A);
  tempB = getTemp(THERMISTOR_PIN_B);
  
  
  // Has to run outside interrupt
  if (do_switch != last) {
    if (do_switch) {
      // Todo: Switch files
      Serial.println("Switch files.");
      do_switch = false;
    }
    last = do_switch;
  }
  
  // Thermistor A
  Serial.print(tempA, 1);
  Serial.print(" ");
  
  // Thermistor B
  Serial.println(tempB, 1);
  
  // Wait one second.
  delay(1000);  
}
