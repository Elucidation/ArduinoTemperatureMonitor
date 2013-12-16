#include <Wire.h>
#include <LiquidCrystal_I2C.h>

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

// Signal LEDs
#define LED_GREEN 13
#define LED_RED 12
// Logic : If Temp A is < 0c, RED
// If Temp A > 5c, GREEN

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// Thermistor sample array
int samples[NUMSAMPLES];

// Used in getTemp to get steinhart value for temperature conversion from thermistor reading.
float steinhart;

// Keeps track of whether LCD backlight should be on or off via interrupt.
volatile bool do_backlight = false;

// Keeps track of last backlight status
bool last = do_backlight;
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

// Switches do_backlight on pushbutton press
void switch_backlight() {
  do_backlight = !do_backlight;
}

////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  // Initialize the lcd 
  lcd.init();
  lcd.print("Initializing...");
  switch_backlight();
  
  // Trigger backlight on push button on pin 2 (interrupt 0)
  attachInterrupt(0, switch_backlight, RISING);
  
  // Set up Signal LEDs
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  
}

void loop() {
  // Read thermistors
  tempA = getTemp(THERMISTOR_PIN_A);
  tempB = getTemp(THERMISTOR_PIN_B);
  
  // Signal LEDS
  // A
  // <= 0 ~ RED
  // 0 < 5 < ~ Nothing
  // >= 5 ~ GREEN
  if (tempA <= 0) {
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, LOW);
  }
  else if (tempA >= 5) {
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, HIGH);
  }
  else {
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
  }
  
  // Has to run outside interrupt
  // TODO : Make faster than 1Hz
  if (do_backlight != last) {
    // Set LCD backlight based on state
    do_backlight ? lcd.backlight() : lcd.noBacklight();
    last = do_backlight;
  }
  
  // Thermistor A
  lcd.setCursor(0,0);
  lcd.print("OUT: ");
  lcd.print(tempA, 1);
  lcd.print((char)223);
  lcd.print("C    ");
  
  // Thermistor B
  lcd.setCursor(0,1);
  lcd.print(" IN: ");
  lcd.print(tempB, 1);
  lcd.print((char)223);
  lcd.print("C    ");
  
  // Wait one second.
  delay(1000);  
}
