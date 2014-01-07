#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ThermistorSensor.h>
#include <SD.h>
/*
  SD card read/write circuit:
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4
*/
// Max filename size in characters
// FAT file systems have a limitation when it comes to naming conventions. 
// You must use the 8.3 format, 
// so that file names look like “NAME001.EXT”, where “NAME001” is an 8 character or fewer string,
// and “EXT” is a 3 character extension. 
// People commonly use the extensions .TXT and .LOG. It is possible to have a shorter file name 
// (for example, mydata.txt, or time.log), but you cannot use longer file names. Read more on the 8.3 convention.
// Any longer than 8 chars + 4 + terminator = 13, and it will start erroring when opening files.
#define MAX_FILENAME_LENGTH 13

// Max index for a log file, ex. log1000.txt given 1000 for example
#define MAX_IDX 1000

// Thermistor Setup
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
// If File error red/green don't ping

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// Disabled backlight tracking - subsumed for file reset control
// Keeps track of whether LCD backlight should be on or off via interrupt.
// volatile bool do_backlight = false;

// Keeps track of last backlight status
// bool last = do_backlight;

// SD Card log file init
File logFile;
String filename;
char filename_buffer[MAX_FILENAME_LENGTH];
int fileIndex = 0;

boolean has_SD_card = false;
boolean has_init = false;


// Thermistor objects and temperature variables
ThermistorSensor thermistorA(THERMISTOR_PIN_A);
ThermistorSensor thermistorB(THERMISTOR_PIN_B);
float tempA, tempB;

bool last, do_switch = false;

void doFileSwitch() {
  // Enable switch files in loop
  do_switch = true;
}

// Switches to next incremented log file
void switchFiles() {
  if (!has_SD_card) {
    loadSD();
  } 
  else {
    Serial.println("Switching files...");
    // Close current file
    logFile.close();
    Serial.print("Closed logfile ");
    Serial.println(filename_buffer);
    
    // Go to next index
    fileIndex++;
    
    // Open new log file
    openLogFile(fileIndex);
    
    Serial.print("Started writing to logfile ");
    Serial.println(filename_buffer);
  }
}

// Gets filename given an index into filename and filename_buffer global variables
// Since we reserved same space for string filename and buffer no check for size
void getFilename(int i) {
  // log%d.txt where %d is integer i
  filename = "log";
  filename += i;
  filename += ".txt";
  // Load filename to buffer (+ \0 terminator)
  filename.toCharArray(filename_buffer, filename.length()+1 );
}

// Returns index of first unused file or maxIndex
int firstUnusedIndex(int index, int maxIndex) {
  while (index < maxIndex) {
    getFilename(index);
    if (!SD.exists(filename_buffer)) {break;}
    index++;
  }
  return index;
}

void openLogFile(int index) {
  getFilename(index);
  logFile = SD.open(filename_buffer, FILE_WRITE);
  if (!logFile) {
    // if the file didn't open, print an error:
    Serial.print("ERROR: Could not open ");
    Serial.println(filename_buffer);
    lcd.clear();
    lcd.print("LOG OPEN ERROR:");
    lcd.setCursor(0,1);
    lcd.print(filename_buffer);
    delay(2000);
    lcd.clear();
  } else {
    lcd.clear();
    lcd.print("USING LOG:");
    lcd.setCursor(0,1);
    lcd.print(filename_buffer);
    delay(2000); // Pause 2 seconds
    lcd.clear();
  }
}

boolean initSD() {
  Serial.print("Initializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
  pinMode(10, OUTPUT);
  
  if (!SD.begin(4)) {
    Serial.println("SD Initialization failed!");
    return false;
  }
  Serial.println("SD Initialization done.");
  return true;
}

void loadSD() {
  // Init SD Card
  if (!initSD()) {
    Serial.println("Failed to Initialize SD Card.");
    has_SD_card = false;
    // return; // Go on but no logging
    lcd.clear();
    lcd.print("ERROR: FAILED");
    lcd.setCursor(0,1);
    lcd.print("NO SD CARD         ");
    delay(2000);
    lcd.clear();
  } else {
    // Card loaded successfully
    has_SD_card = true;
    lcd.clear();
    lcd.print("SUCCESS:");
    lcd.setCursor(0,1);
    lcd.print("INIT SD CARD");
    delay(2000);
    lcd.clear();
    fileIndex = firstUnusedIndex(0, MAX_IDX); // Filename counter "log%d.txt" %d <-- i
    openLogFile(fileIndex);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  // Initialize the lcd with backlight on
  lcd.init();
  lcd.backlight();
  lcd.print("Initializing...");

  // Reserve space for filename string
  filename.reserve(MAX_FILENAME_LENGTH);
  
  // Start serial
  Serial.begin(9600);
  
  loadSD();
  
  // Trigger backlight on push button on pin 2 (interrupt 0)
  attachInterrupt(0, doFileSwitch, RISING);
  
  // Initialize succeeded
  has_init = true;
}

void loop() {
  if (!has_init) {return;} // Don't do anything unless initialization succeeded
  
  // Read thermistors
  tempA = thermistorA.getReading();
  tempB = thermistorB.getReading();
  
  // Signal LEDS
  // A
  // <= 0 ~ RED
  // 0 < 5 < ~ Nothing
  // >= 5 ~ GREEN
  if (!(has_SD_card && logFile)) {
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, HIGH);
  }
  else if (tempA <= 0) {
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
  if (do_switch != last) {
    if (do_switch) {
      // Todo: Switch files
      switchFiles();
      
      do_switch = false;
    }
    last = do_switch;
  }
  
  // Log temperatures if logfile exists 
  if (has_SD_card && logFile) {
    // Thermistor A & B
    Serial.print(millis());
    Serial.print(" ");
    Serial.print(tempA, 1);
    Serial.print(" ");
    Serial.println(tempB, 1);
    
    logFile.print(millis()); // Number of milliseconds since the program started (unsigned long)
    logFile.print(" ");
    logFile.print(tempA, 1); // Thermistor A (celsius)
    logFile.print(" ");
    logFile.println(tempB, 1); // Thermistor B (celsius)
    logFile.flush();
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
