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
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "oled_stuff.h"

#define ONEWIRE_PIN 10
#define CARDCS 4
#define MAX_FILES 999

Adafruit_SSD1306 oled = Adafruit_SSD1306();
OneWire onewireBus(ONEWIRE_PIN);
DallasTemperature ds18(&onewireBus);

float tempF;
int dataFileCounter;
char fileName[13] = {' '};

//--------------------------------------------------------------------------------------
//                               S  E  T  U  P
//--------------------------------------------------------------------------------------
void setup() {  
  //while (!Serial);  
  Serial.begin(9600);
  Serial.println(F("Snow Logger ****"));

  initStuff();
  dataFileCounter = 0;
  Serial.print(F("Counting files..."));  
  countFiles(SD.open("/"), 0);
  Serial.println(dataFileCounter);    
}

//--------------------------------------------------------------------------------------
//                                L  O  O  P
//--------------------------------------------------------------------------------------
void loop() {
  oled.clearDisplay();
  tempF = getTemperature();  
  oled.setCursor(65,0);
  oled.print(tempF);
  oled.setCursor(65,18);
  oled.print(dataFileCounter);
  if (buttonA()) oled.print("A");
  if (buttonB()) oled.print("B");
  if (buttonC()) oled.print("C");
  oled.display();
  delay(100);
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
    Serial.println(F("SD failed, or not present"));
    oled.print(F("SD FAILED"));
    oled.display();
    while (1);
  }
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

