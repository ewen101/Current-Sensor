
#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <TimeLib.h>
#include <Adafruit_INA219.h>

#define txPin 2
#define ONE_SECOND 1000

Adafruit_INA219 ina219;

time_t startTime;
File dataFile;
SoftwareSerial LCD = SoftwareSerial(0, txPin);

const int LCDdelay=10;  // conservative, 2 actually works

// wbp: goto with row & column
void lcdPosition(int row, int col) {
  LCD.write(0xFE);   //command flag
  LCD.write((col + row*64 + 128));    //position 
  delay(LCDdelay);
}
void clearLCD(){
  LCD.write(0xFE);   //command flag
  LCD.write(0x01);   //clear command.
  delay(LCDdelay);
}
void backlightOn() {  //turns on the backlight
  LCD.write(0x7C);   //command flag for backlight stuff
  LCD.write(157);    //light level.
  delay(LCDdelay);
}
void backlightNormal() {  //turns on the backlight
  LCD.write(0x7C);   //command flag for backlight stuff
  LCD.write(130);    //light level.
  delay(LCDdelay);
}
void backlightOff(){  //turns off the backlight
  LCD.write(0x7C);   //command flag for backlight stuff
  LCD.write(128);     //light level for off.
   delay(LCDdelay);
}
void serCommand(){   //a general function to call the command flag for issuing all other commands   
  LCD.write(0xFE);
}

//SerialLCD Functions
void selectLineOne(){  //puts the cursor at line 0 char 0.
   LCD.write(0xFE);   //command flag
   LCD.write(128);    //position
}
void selectLineTwo(){  //puts the cursor at line 2 char 0.
   LCD.write(0xFE);   //command flag
   LCD.write(192);    //position
}

void goTo(int position) { //position = line 1: 0-19, line 2: 20-39, etc, 79+ defaults back to 0
  if (position<20){
    Serial.write(0xFE);   //command flag
    Serial.write((position+128));    //position
    }
  else if (position<40){
    Serial.write(0xFE);   //command flag
    Serial.write((position+128+64-20));    //position 
    } 
  else { goTo(0); }
}


void setup(void) {
  Serial.begin(115200);
  while (!Serial) {
      // will pause Zero, Leonardo, etc until serial console opens
      delay(1);
      Serial.println("Trying to connect to chip...");
  }

  Serial.print("Initializing SD card...");

  if (!SD.begin(8)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  // Deletes the file if it already exists
  dataFile = SD.open("dataFile.csv", FILE_WRITE);
  dataFile.close();
  SD.remove("DATAFILE.csv");

  dataFile = SD.open("dataFile.csv", FILE_WRITE);
  dataFile.println("Time(s), current(mA)");
  dataFile.close();

  uint32_t currentFrequency;
    
  Serial.println("Hello!");
  
  // Initialize the INA219.
  // By default the initialization will use the largest range (32V, 2A).  However
  // you can call a setCalibration function to change this range (see comments).
  
  ina219.begin();
  
  // To use a slightly lower 32V, 1A range (higher precision on amps):
  //ina219.setCalibration_32V_1A();
  // Or to use a lower 16V, 400mA range (higher precision on volts and amps):
  ina219.setCalibration_16V_400mA();

  Serial.println("Measuring voltage and current with INA219 ...");

  //LCD initialization
  pinMode(txPin, OUTPUT);
  LCD.begin(9600);
  backlightOn();
  clearLCD();
  lcdPosition(0,0);
  LCD.print("Hello world!");

  delay(ONE_SECOND);

  startTime = now();
}


void loop(void) {
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;

  time_t currentTime = now();
  
  //for serial port for IDE
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);

  dataFile = SD.open("dataFile.csv", FILE_WRITE);
  
  Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
  Serial.print("Power:         "); Serial.print(power_mW); Serial.println(" mW");
  Serial.println("");

  //resets LCD
  clearLCD();
  lcdPosition(0,0);

//For Voltage
  selectLineOne();
  LCD.print("Load Vtg: ");
  LCD.print(loadvoltage);
  LCD.print(" V");

//For Power
//selectLineOne();
//LCD.print("Power: ");
//LCD.print(power_mW);
//LCD.print(" mW");

  selectLineTwo();
  LCD.print("Current: ");
  LCD.print(current_mA);
  LCD.print(" mA");

  if (dataFile) {
    dataFile.print(currentTime - startTime);
    dataFile.print(",");
    dataFile.print(current_mA);
    dataFile.print("\n");
    Serial.println("Printed to SD");
  }
  else {
    Serial.println("error opening file in SD card");
  }

  dataFile.close();

  delay(ONE_SECOND*0.5); // Half a Second

  Serial.print("\n");
}
