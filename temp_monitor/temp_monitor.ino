#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ThermistorSensor.h>

// which analog pins to connect
#define THERMISTOR_PIN_A A0
#define THERMISTOR_PIN_B A1

// PIN 2 - Used for Pushbutton with interrupt 0

// If we want serial output
#define SERIAL_OUTPUT

// Signal LEDs
#define LED_GREEN 13
#define LED_RED 12
// Logic : If Temp A is < 0c, RED
// If Temp A > 5c, GREEN

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// Keeps track of whether LCD backlight should be on or off via interrupt.
volatile bool do_backlight = false;

// Keeps track of last backlight status
bool last = do_backlight;

// Thermistor objects and temperature variables
ThermistorSensor thermistorA(THERMISTOR_PIN_A);
ThermistorSensor thermistorB(THERMISTOR_PIN_B);
float tempA, tempB;

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
  
  #ifdef SERIAL_OUTPUT
  // Start serial
  Serial.begin(9600);
  #endif
}

void loop() {
  // Read thermistors
  tempA = thermistorA.getReading();
  tempB = thermistorB.getReading();
  
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

  #ifdef SERIAL_OUTPUT
  Serial.print("OUT: ");
  Serial.print(tempA, 1);
  Serial.print("C\t");
  
  // Thermistor B
  // Serial.setCursor(0,1);
  Serial.print("IN: ");
  Serial.print(tempB, 1);
  Serial.println(" C");
  #endif
  
  // Wait one second.
  delay(1000);  
}
