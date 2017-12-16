//--------------------------------------------------------------------------------------
// Snow Logger
//
// 2017-12-14
// Carter Nelson
//--------------------------------------------------------------------------------------
#include <SD.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_SSD1306.h>
#include "oled_stuff.h"

#define ONEWIRE_PIN 18
#define CARDCS 4
#define MAX_FILES 999
#define MAX_DATA_POINTS 37
#define DEBOUNCE 250
#define DISPLAY_RATE 100

Adafruit_SSD1306 oled = Adafruit_SSD1306();
OneWire onewireBus(ONEWIRE_PIN);
DallasTemperature ds18(&onewireBus);

File dataFile;
float temperature;
int dataFileCounter, dataPointCounter;
int lastUpdate;
char* fileName = "SNOWXXX.DAT";
enum mode {
  STOPPED,
  LOGGING,
} currentMode = STOPPED;

//--------------------------------------------------------------------------------------
//                               S  E  T  U  P
//--------------------------------------------------------------------------------------
void setup() {  
  //while (!Serial);  
  Serial.begin(9600);
  Serial.println(F("Snow Logger ****"));

  initStuff();
  Serial.print(F("Counting files..."));  
  countFiles(SD.open("/"), 0);
  Serial.println(dataFileCounter);    
}

//--------------------------------------------------------------------------------------
//                                L  O  O  P
//--------------------------------------------------------------------------------------
void loop() {
  // Get current temperature.
  temperature = getTemperature();

  // Start logging / take a data point
  if (buttonA()) {
    oled.setCursor(50,18);
    oled.print("o");
    oled.display();

    if (currentMode != LOGGING) {
      openNewFile(dataFileCounter++);
    }
    currentMode = LOGGING;

    Serial.print(dataPointCounter); Serial.print(" "); Serial.println(temperature);
    dataFile.print(dataPointCounter); dataFile.print(" "); dataFile.println(temperature);

    dataPointCounter++;    
    if (dataPointCounter >= MAX_DATA_POINTS) {
      stopLogging();
    }
    delay(DEBOUNCE);
  }

  // Abort
  if (buttonC() && currentMode == LOGGING) {
    stopLogging();
  }

  // Update display
  if (millis() - lastUpdate > DISPLAY_RATE) {
    updateDisplay();
  }
}

//--------------------------------------------------------------------------------------
void stopLogging() {
  Serial.println("Closing file.");
  closeFile();
  dataPointCounter = 0;
  currentMode = STOPPED;        
}

//--------------------------------------------------------------------------------------
void openNewFile(int counter) {
  // Not enough room for sprintf!!!
  fileName[4] = 0x30 + dataFileCounter / 100 % 10;
  fileName[5] = 0x30 + dataFileCounter / 10  % 10;  
  fileName[6] = 0x30 + dataFileCounter       % 10;
  Serial.print("Opening file..."); Serial.println(fileName);
  dataFile = SD.open(fileName, FILE_WRITE);
}

//--------------------------------------------------------------------------------------
void closeFile() {
  dataFile.flush();
  dataFile.close();
}

//--------------------------------------------------------------------------------------
void updateDisplay() {
  oled.clearDisplay();
  //
  // Stuff on the right side
  //
  oled.setCursor(65,0);
  oled.print(temperature);
  oled.setCursor(65,18);
  oled.print(dataFileCounter);

  //
  // Mode base info
  //
  oled.setCursor(0,0);
  switch (currentMode) {
    case STOPPED:
      oled.print("PRESS");
      oled.setCursor(0,18); 
      oled.print("  A");
      break;
    case LOGGING:
      oled.print("POINT");
      oled.setCursor(0,18); 
      oled.print(dataPointCounter);
      break;
    default:
      oled.print("????");
  }
  
  oled.display();
}

//--------------------------------------------------------------------------------------
void initStuff() {
  //
  // OLED
  //
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // Splash screen for warm fuzzy
  oled.display();
  delay(1000);

  oled.clearDisplay();
  oled.display();
  oled.setTextSize(2);
  oled.setTextColor(WHITE);
  oled.setCursor(0,0);

  //
  // Buttons
  //
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);  

  //
  // SD Card
  //
  if (!SD.begin(CARDCS)) {
    Serial.println("SD failed, or not present");
    oled.print("SD FAILED");
    oled.display();
    while (1);
  }

  //
  // Misc.
  //
  dataFileCounter = 0;
  dataPointCounter = 0;
  lastUpdate = millis();
}

//--------------------------------------------------------------------------------------
float getTemperature() {
  ds18.requestTemperatures(); 
  return ds18.getTempFByIndex(0);
}

//--------------------------------------------------------------------------------------
void countFiles(File dir, int level) {
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) return;
    if (entry.isDirectory()) {
      // Recursive call to scan next dir level
      countFiles(entry, level + 1);
    } else {
      // Only count files in root dir
      if (level == 0) {
        dataFileCounter++;
      }
    }
    entry.close();
    // Stop scanning if we hit max
    if (dataFileCounter >= MAX_FILES) return;
  } 
}

//--------------------------------------------------------------------------------------
bool buttonA() {
  return ! digitalRead(BUTTON_A);
}

//--------------------------------------------------------------------------------------
bool buttonB() {
  return ! digitalRead(BUTTON_B);
}

//--------------------------------------------------------------------------------------
bool buttonC() {
  return ! digitalRead(BUTTON_C);
}

