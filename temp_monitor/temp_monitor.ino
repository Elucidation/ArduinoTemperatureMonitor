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

// SD Card log file init
File logFile;
String filename;
char filename_buffer[MAX_FILENAME_LENGTH];
int fileIndex = 0;

// Thermistor sample array
int samples[NUMSAMPLES];

// Used in getTemp to get steinhart value for temperature conversion from thermistor reading.
float steinhart;

// Thermistor A & B Temperature variables
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
  // Enable switch files in loop
  do_switch = true;
}

// Switches to next incremented log file
void switchFiles() {
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
  Serial.print("  Trying to open '");
  Serial.print(filename_buffer);
  Serial.println("'");
  logFile = SD.open(filename_buffer, FILE_WRITE);
  if (!logFile) {
    // if the file didn't open, print an error:
    Serial.print("ERROR: Could not open ");
    Serial.println(filename_buffer);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////
boolean has_init = false;
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
void setup() {
  // Reserve space for filename string
  filename.reserve(MAX_FILENAME_LENGTH);
  
  // Start serial
  Serial.begin(9600);
  
  // Init SD Card
  if (!initSD()) {
    Serial.println("Failed to Initialize SD Card.");
    return;
  }
  
  fileIndex = firstUnusedIndex(0, MAX_IDX); // Filename counter "log%d.txt" %d <-- i
  openLogFile(fileIndex);
  
  // Trigger backlight on push button on pin 2 (interrupt 0)
  attachInterrupt(0, doFileSwitch, RISING);
  
  // Initialize succeeded
  has_init = true;
}

void loop() {
  if (!has_init) {return;} // Don't do anything unless initialization succeeded
  
  // Read thermistors
  tempA = getTemp(THERMISTOR_PIN_A);
  tempB = getTemp(THERMISTOR_PIN_B);
  
  
  // Has to run outside interrupt
  if (do_switch != last) {
    if (do_switch) {
      // Todo: Switch files
      Serial.println("Switching files...");
      switchFiles();
      
      do_switch = false;
    }
    last = do_switch;
  }
  
  // Log temperatures if logfile exists 
  if (logFile) {
    // Thermistor A & B
    Serial.println("Logging.");
    logFile.print(tempA, 1);
    logFile.print(" ");
    logFile.println(tempB, 1);
  }
  
  // Wait one second.
  delay(1000);  
}
