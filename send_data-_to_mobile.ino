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

TaskHandle_t gpsTaskHandle = NULL;
TaskHandle_t gyroTaskHandle = NULL;

void gyroTask(void *pvParameters);
void gpsTask(void *pvParameters);



void setup()
{
  Serial.begin(9600);
  gpsSerial.begin(9600);
  gprsSerial.begin(9600);
  Wire.begin();
  mpu.initialize();

  // Create tasks on different cores
  xTaskCreatePinnedToCore(
      gyroTask,           /* Function to implement the task */
      "GyroTask",         /* Name of the task */
      10000,              /* Stack size in words */
      NULL,               /* Task input parameter */
      1,                  /* Priority of the task */
      &gyroTaskHandle,    /* Task handle. */
      1);                 /* Core where the task should run */

  xTaskCreatePinnedToCore(
      gpsTask,            /* Function to implement the task */
      "GPSTask",          /* Name of the task */
      10000,              /* Stack size in words */
      NULL,               /* Task input parameter */
      1,                  /* Priority of the task */
      &gpsTaskHandle,     /* Task handle. */
      0);                 /* Core where the task should run */
}
void loop(){

}



void gyroTask(void *pvParameters)
{
  
  for (;;)
  {
    readSensorData();
    
  }
}

void gpsTask(void *pvParameters)
{
  
  for (;;)
  {
     
unsigned long currentMillis = millis();

  if (currentMillis - lastInternetCheckTime >= internetCheckInterval)
  {
    internet_connect();
    lastInternetCheckTime = currentMillis;
  }

  if (checkInternetConnectivity())
  {
    Serial.println("Internet is available");
    gyro_data();
    //sim_response();

    // Check for alert conditions
    if (checkForAlert())
    {
      // Trigger alert actions
      sendAlertSMS();
    }

    sendDataToServer();
  }
  else
  {
    Serial.println("Internet is not available");
  }
  }
}



void sendAlertSMS()
{
  // Construct a message with all the data
  String alertMessage = "$";
  alertMessage += "EPB" ;
  alertMessage += ",";
  alertMessage += "EMR";
  alertMessage += ",";
  alertMessage += "865525351624256";
  alertMessage += ",";
  alertMessage += "NM";
  alertMessage += ",";
  alertMessage += String(dateString);
  alertMessage += ",";
  alertMessage +=  String(gps.location.lat()>0 ? "A" : "V");
  alertMessage += ",";
  alertMessage += String(gps.location.lat());
  alertMessage += ",";
  alertMessage += String( (longitude >= 0) ? "E" : "W");
  alertMessage += ",";
  alertMessage +=String(gps.location.lng());
  alertMessage += ",";
 alertMessage += String((latitude >= 0) ? "N" : "S");
 alertMessage += ",";
 alertMessage += String(gps.altitude.meters());
 alertMessage += ",";
 alertMessage += String(gps.speed.kmph());
  alertMessage += ",";
   alertMessage += "+918827774411";
    //alertMessage += ",";
     alertMessage += "*";
 

  // ... Include other data fields ...

  // Send SMS to the specified mobile number
  sendSMS("+919109802925", alertMessage);
}

bool checkForAlert()
{
  // Define your alert conditions here
  // For example, if altitude is above a certain threshold
  return true; // Adjust the threshold as needed
}

// ... Other functions ...

void sendSMS(const String &phoneNumber, const String &message)
{
  gprsSerial.println("AT+CMGF=1"); // Set SMS mode to text
  delay(1000);

  gprsSerial.print("AT+CMGS=\"");
  gprsSerial.print(phoneNumber);
  gprsSerial.println("\"");
  delay(1000);

  gprsSerial.print(message);
  delay(100);
  gprsSerial.write(26); // Ctrl+Z to send SMS
  delay(1000);
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

   
}

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
 // jsonDocument.clear();
 jsonDocument["Altitude"] = altitude;
 jsonDocument["PDOP"] = 1.25;
 jsonDocument["HDOP"] = HDOP;
 jsonDocument["NoOfSatellites"] = numSatellites;
 jsonDocument["Heading"] = course;
 jsonDocument["GPSfix"] = gps_fix;;
  jsonDocument["acceloX"] = AcceloX;
  jsonDocument["acceloY"] = AcceloY;
  jsonDocument["acceloZ"] = AcceloZ;
  jsonDocument["gyroX"] = gyroX;
  jsonDocument["gyroY"] = gyroY;
  jsonDocument["gyroZ"] = gyroZ;
  jsonDocument["temperature"] = temp;
  jsonDocument["Longitude"] = longitude;
  jsonDocument["LongitudeDirection"] = longitudeDirection;
  jsonDocument["Latitude"] = latitude;
  jsonDocument["LatitudeDirection"] = latitudeDirection;
  jsonDocument["Speed"] = speed;
  jsonDocument["GSMSignalStrength"] = signalstrength;
 jsonDocument["MCC"] = mcc;
 jsonDocument["MNC"] =mnc;
 jsonDocument["LAC"] = lac;
  jsonDocument["CellId"] = cellid;
 jsonDocument["NMR"] = nmr;
 jsonDocument["NetworkOperatorName"] = operatorname;
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
   ShowSerialData();

   gprsSerial.println("AT+HTTPDATA=" + String(sendtoserver.length()) + ",100000");
   Serial.println(sendtoserver);
   delay(1000);
   ShowSerialData();

   gprsSerial.println(sendtoserver);
   delay(1000);
   ShowSerialData();

   gprsSerial.println("AT+HTTPACTION=1");
   delay(1000);
   ShowSerialData();

   gprsSerial.println("AT+HTTPREAD");
   delay(1000);
  ShowSerialData();

   gprsSerial.println("AT+HTTPTERM");
   delay(1000);
   ShowSerialData();
 
}


void readSensorData() {
  // Read GPS data
  while (gpsSerial.available()) {
    if (gps.encode(gpsSerial.read())) {
        gps_fix=gps.location.lat()>0 ? 1 : 0;
        numSatellites = gps.satellites.value();
        latitude = gps.location.lat();
        longitude = gps.location.lng();
        altitude = gps.altitude.meters();
        speed = gps.speed.kmph();
        course=gps.course.deg();
        day = gps.date.day();
        month = gps.date.month();
        year = gps.date.year();
        hour = gps.time.hour();
        minute = gps.time.minute();
        second = gps.time.second();
        dateString = String(day) + String(month) + String(year);
        TimeString = String(hour) + String(minute) + String(second);
        HDOP=gps.hdop.hdop();
        
      

        // Determine longitude direction
        longitudeDirection = (longitude >= 0) ? "E" : "W";
        latitudeDirection = (latitude >= 0) ? "N" : "S";

        
      
    }
  }
}
bool checkInternetConnectivity() {
  gprsSerial.println("AT+CGATT?");
  delay(1000);
  String reply = "";
  Serial.println(reply);
  while ( gprsSerial.available()) {
    reply +=  gprsSerial.readStringUntil('\n');
  }
  return reply.indexOf("+CGATT: 1") != -1;  // Returns true if GPRS is attached (internet is available)
}

void ShowSerialData()
{
  while( gprsSerial.available()!=0)
  Serial.write( gprsSerial.read());
  //delay(2000); 
  
}
