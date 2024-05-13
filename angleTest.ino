#include "Wire.h"
#include <MPU6050_light.h>
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

/*
PHONE WIRES/PINS:
- BLUE = V_cc
- WHITE = GND
- RED = A5 (SCL)
- PURPLE = A4 (SDA)
*/

MPU6050 mpu(Wire);
unsigned long timer = 0;

const float sensorMin = -180;      // Minimum sensor value (degrees)
const float sensorMax = 180;       // Maximum sensor value (degrees)

float sensorValue;                 // value read directly from sensor (processed by MPU6050 library)
int mappedValue;                   // value of MIDI content

float previousSensorValue;         // used to avoid sending useless MIDI messages if sensor value doesn't change

const int resetCoordinatesBtn = 2; // button used for re-calibration of sensor coordinates
const int ledPin = 13;
float baseValue = 0;               // coordinate offset
int resetBtnState;

void setup() {
  initAll();
}

void loop() {
  sendCC();
  resetCoords();
}

// Initialization 
void initAll(){
  Serial.begin(9600);
  Wire.begin();
  byte status = mpu.begin();
  // Serial.print(F("MPU6050 status: "));
  // Serial.println(status);
  // while(status!=0){ } // stop everything if could not connect to MPU6050
  // Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  mpu.upsideDownMounting = true; // uncomment this line if the MPU6050 is mounted upside-down
  mpu.calcOffsets(); // gyro and accelero
  // Serial.println("Done!\n");
  MIDI.begin();
  pinMode(ledPin, OUTPUT);
  pinMode(resetCoordinatesBtn, INPUT);
}

// Converts sensor data into MIDI CC messages

void sendCC(){
  mpu.update();
  
  // if((millis()-timer)>100){ // print data every 10ms
    // Serial.print("X : ");
    // Serial.print(mpu.getAngleX());
    // Serial.print("\tY : ");
    // Serial.print(mpu.getAngleY());
    // Serial.print("\tZ : ");
    // Serial.println(mpu.getAngleZ());
      

  // Get sensor value
  // NOTE: X and Y angles return weird values, maybe try different library
  sensorValue = mpu.getAngleZ() + baseValue;
  // Serial.print("Sensor value");
  // Serial.print(sensorValue);
  // Serial.print("\n");

  // Clip sensor value if limits are exceeded
  if(sensorValue < sensorMin)
    sensorValue = sensorMin;
  else if(sensorValue > sensorMax)
    sensorValue = sensorMax;
  

  // Map value to MIDI range
  mappedValue = map(sensorValue, sensorMin, sensorMax, 0, 127);

  // Serial.print("mappedValue:");
  // Serial.print(mappedValue);
  // Serial.print("\n");
  
  if(mappedValue != previousSensorValue){
    MIDI.sendControlChange(1, mappedValue, 1); // (CC mode, value, MIDI channel); CC mode = 1 corresponds to modulation wheel
    previousSensorValue = mappedValue;
    // Serial.print("Sent new value");
    // Serial.print(mappedValue);
    // Serial.print("\n");
  }
  delay(100);
}

// NEEDS REVISION
void resetCoords(){
  resetBtnState = digitalRead(resetCoordinatesBtn);
  
  if(resetBtnState == HIGH){
    mpu.calcOffsets();            // re-calibrate coordinates
    digitalWrite(ledPin, HIGH);
    delay(1000);                  // provide 1 second to allow re-calibration
    digitalWrite(ledPin, LOW);
  }
}


