#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <SPIMemory.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <MPU6050.h>
#include <ArduinoJson.h>
#define rxGPS 33
#define txGPS 32
MPU6050 mpu;
SoftwareSerial  gprsSerial(17,16);
SoftwareSerial gpsSerial(rxGPS,txGPS);
TinyGPSPlus gps;
int numSatellites;
int gps_fix;
float latitude;
float longitude;
float altitude;
float speed;
float PDOP,HDOP;
int day, month, year;
int hour, minute, second;
String TimeString;
String dateString;
String latitudeDirection;
String longitudeDirection;
String course;
String response;
String operatorname;
String signalstrength;
String imei;
String mcc;
String mnc;
String cellid;
String nmr;
String lac;
String LACcell;
String mccMnc;


int lastInternetCheckTime = 0;
int internetCheckInterval = 20000; 
int16_t rawGyroX, rawGyroY, rawGyroZ;
float gyroX, gyroY, gyroZ;
int16_t rawAcceloX, rawAcceloY, rawAcceloZ;
float AcceloX, AcceloY, AcceloZ;
int16_t rawTemp;
float temp;


void setup(){
Serial.begin(9600);
}
byte calculateChecksum(const byte* data, size_t length) {
  byte checksum = 0;
  for (size_t i = 0; i < length; i++) {
    checksum ^= data[i];
  }
  return checksum;
}
void loop(){
DynamicJsonDocument jsonDocument(1024);

  // Add your data to the JSON document
  jsonDocument["Altitude"] = altitude;
  jsonDocument["PDOP"] = 1.25;
  jsonDocument["HDOP"] = HDOP;
  // Add other data fields

  // Calculate checksum for the JSON data
  size_t bufferSize = measureJson(jsonDocument);
  char buffer[bufferSize];
  serializeJson(jsonDocument, buffer, bufferSize);

  // Calculate checksum for the serialized JSON data
  byte checksum = calculateChecksum((byte*)buffer, bufferSize);


  // Add checksum to the JSON document
  jsonDocument["Checksum"] = checksum;
  String sendtoserver;
  serializeJson(jsonDocument, sendtoserver);
  Serial.println(sendtoserver);

}