#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <SPIMemory.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <MPU6050.h>
#include <ArduinoJson.h>
#define rxGPS 32
#define txGPS 33
MPU6050 mpu;
SoftwareSerial  gprsSerial(17,16);
SoftwareSerial gpsSerial(rxGPS,txGPS);
TinyGPSPlus gps;
SemaphoreHandle_t gpsSemaphore;
unsigned long lastGpsProcessTime = 0;
const unsigned long gpsProcessInterval = 10000;
bool gpsProcessed = false;
int numSatellites;
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

int lastInternetCheckTime = 0;
int internetCheckInterval = 20000; 
int16_t rawGyroX, rawGyroY, rawGyroZ;
float gyroX, gyroY, gyroZ;
int16_t rawAcceloX, rawAcceloY, rawAcceloZ;
float AcceloX, AcceloY, AcceloZ;
int16_t rawTemp;
float temp;
// DynamicJsonDocument jsonDocument(512);
void setup()
{ Serial.begin(9600);
  gpsSerial.begin(9600);
  gprsSerial.begin(9600); 
  Wire.begin();
  mpu.initialize();             // the GPRS baud rate     
  delay(1000);
}
 //_______________________________________________________________________________________________

 void internet_connect(){

 if ( gprsSerial.available())
    Serial.write( gprsSerial.read());

   gprsSerial.println("AT");
   delay(1000);

   gprsSerial.println("AT+CGATT?");
   delay(1000);

   gprsSerial.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
   delay(1000);
   //ShowSerialData();

   gprsSerial.println("AT+SAPBR=3,1,\"APN\",\"airtelgprs.com\"");
   delay(1000);
  // ShowSerialData();

   gprsSerial.println("AT+SAPBR=1,1");
   delay(1000);
  // ShowSerialData();

   gprsSerial.println("AT+SAPBR=2,1");
   delay(1000);
   //ShowSerialData();

   gprsSerial.println("AT+HTTPINIT");
   delay(1000);
   //ShowSerialData();

   gprsSerial.println("AT+HTTPPARA=\"CID\",1");
   delay(1000);
   //ShowSerialData();
  
}

void sendDataToServer(){
  
  
  DynamicJsonDocument jsonDocument(1024);
  jsonDocument.clear();
  jsonDocument["acceloX"] = AcceloX;
  jsonDocument["acceloY"] = AcceloY;
  jsonDocument["acceloZ"] = AcceloZ;
  jsonDocument["gyroX"] = gyroX;
  jsonDocument["gyroY"] = gyroY;
  jsonDocument["gyroZ"] = gyroZ;
  jsonDocument["Temperature"] = temp;
  jsonDocument["Longitude"] = longitude;
 // jsonDocument["longitudeDirection"] = longitudeDirection;
  jsonDocument["Latitude"] = latitude;
  //jsonDocument["latitudeDirection"] = latitudeDirection;
  jsonDocument["speed"] = speed;

  jsonDocument["Time"] = TimeString;
  jsonDocument["Date"] = dateString;

  String sendtoserver;
  serializeJson(jsonDocument, sendtoserver);
  delay(1000);

   gprsSerial.println("AT+HTTPPARA=\"URL\",\"http://65.0.85.221/api/v1/addGyro\"");
   delay(1000);
   //ShowSerialData();

   gprsSerial.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
   delay(1000);
   //uShowSerialData();

   gprsSerial.println("AT+HTTPDATA=" + String(sendtoserver.length()) + ",100000");
   Serial.println(sendtoserver);
   delay(1000);
   //ShowSerialData();

   gprsSerial.println(sendtoserver);
   delay(1000);
  // ShowSerialData();
//
   gprsSerial.println("AT+HTTPACTION=1");
   delay(1000);
   //ShowSerialData();

   gprsSerial.println("AT+HTTPREAD");
   delay(1000);
 // ShowSerialData();

   gprsSerial.println("AT+HTTPTERM");
   delay(1000);
  // ShowSerialData();
 clearSerialBuffer(gprsSerial);
}

void clearSerialBuffer(Stream &serial) {
  while (serial.available()) {
    serial.read();
  }
}
void gyro_data(){
   mpu.getRotation(&rawGyroX, &rawGyroY, &rawGyroZ);
   mpu.getAcceleration(&rawAcceloX, &rawAcceloY, &rawAcceloZ);
   rawTemp = mpu.getTemperature();

   AcceloX = rawAcceloX / 16384.0;
   AcceloY = rawAcceloY / 16384.0;
   AcceloZ = rawAcceloZ / 16384.0;

   gyroX = rawGyroX / 131.0;
   gyroY = rawGyroY / 131.0;
   gyroZ = rawGyroZ / 131.0;

   temp = ((rawTemp) / 340.0) + 36.53;

   delay(2000);
}


void readSensorData() {
  // Read GPS data
  while (gpsSerial.available()) {
    if (gps.encode(gpsSerial.read())) {
      // if (!gpsProcessed) {
      //   gpsProcessed = true;
      gpsSerial.println("AT+QGPSCFG=\"gpsnmeatype\",2");
        numSatellites = gps.satellites.value();
        latitude = gps.location.lat();
        longitude = gps.location.lng();
        altitude = gps.altitude.meters();
        speed = gps.speed.mps();
        day = gps.date.day();
        month = gps.date.month();
        year = gps.date.year();
        hour = gps.time.hour();
        minute = gps.time.minute();
        second = gps.time.second();
        dateString = String(day) + String(month) + String(year);
        TimeString = String(hour) + String(minute) + String(second);
        HDOP=gps.hdop.hdop();
        latitudeDirection = (latitude >= 0) ? "N" : "S";
        

        // Determine longitude direction
        longitudeDirection = (longitude >= 0) ? "E" : "W";
      
    }
  }
  // }
  // Read Gyro data
  gyro_data();
}
bool checkInternetConnectivity() {
  gprsSerial.println("AT+CGATT?");
  delay(1000);
  String response = "";

  while ( gprsSerial.available()) {
    response +=  gprsSerial.readStringUntil('\n');
  }
  return response.indexOf("+CGATT: 1") != -1;  // Returns true if GPRS is attached (internet is available)
}


void loop()
{

  unsigned long currentMillis = millis();
    
  //  Check internet connectivity every 20 seconds
   if (currentMillis - lastInternetCheckTime >= internetCheckInterval) {
     internet_connect();
     lastInternetCheckTime = currentMillis;
   }
   if (checkInternetConnectivity()) {
    Serial.println("Internet is available");
    readSensorData();
    sendDataToServer();
  } else {
    Serial.println("Internet is not available");
  }
}

void ShowSerialData()
{
  while( gprsSerial.available()!=0)
  Serial.write( gprsSerial.read());
 // delay(2000); 
  
}