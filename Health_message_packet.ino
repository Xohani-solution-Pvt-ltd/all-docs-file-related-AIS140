#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <MPU6050.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#define RXD2 32
#define TXD2 33

HardwareSerial gprsSerial(1);  
HardwareSerial gpsSerial(2);  

TaskHandle_t sensorTaskHandle = NULL;
TaskHandle_t sendDataTaskHandle = NULL;
int lastInternetCheckTime = 0;
int internetCheckInterval = 4000;

// Add other global variables here

void sensorTask(void *pvParameters);
void sendDataTask(void *pvParameters);
SemaphoreHandle_t gpsSemaphore;
MPU6050 mpu;

TinyGPSPlus gps;
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
int16_t rawGyroX, rawGyroY, rawGyroZ;
float gyroX, gyroY, gyroZ;
int16_t rawAcceloX, rawAcceloY, rawAcceloZ;
float AcceloX, AcceloY, AcceloZ;
int16_t rawTemp;
float temp;
String valueStr;
String valueStr1;
bool isConnected;
bool messagecome;
String response;
String operatorName;
String signalstrength;
String imei;
String MCCMNC;
String mcc;
String mnc;
String cellId;

String lac;
String LACcell;

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600, SERIAL_8N1, RXD2, TXD2); // Use hardware serial for GPS
  gprsSerial.begin(9600, SERIAL_8N1, 17, 16);  
  Wire.begin();
  mpu.initialize();
  // gprsSerial.println("AT+CMGF=1"); // Set SMS mode to text
  // delay(1000);
   
 // gpsSemaphore = xSemaphoreCreateBinary();

xTaskCreatePinnedToCore(sensorTask, "SensorTask", 10000, NULL, 1, &sensorTaskHandle, 0);
xTaskCreatePinnedToCore(sendDataTask, "SendDataTask", 10000, NULL, 1, &sendDataTaskHandle, 1);

}

//-------------------------------------------------------------------------------------

void loop(){}

//-------------------------------------------------------------------------------------

void sim_response(){

// gprsSerial.println("AT+CMGF=1"); // Configuring TEXT mode
// updateSerial();
// gprsSerial.println("AT+CNMI=1,2,0,0,0"); 
//  updateSerial(); 
gprsSerial.println("AT+COPS?");
 updateSerial();


// gprsSerial.println("AT+CREG?"); 
// updateSerial();

gprsSerial.println("AT+CSQ"); 
updateSerial();

gprsSerial.println("AT+GSN"); 
updateSerial();

gprsSerial.println("AT+CIMI"); 
updateSerial();
gprsSerial.println("AT+CREG=2"); 
updateSerial(); 

gprsSerial.println("AT+CGREG?"); 
updateSerial();

gprsSerial.println("AT+CGED=0"); 
updateSerial();
}

//-------------------------------------------------------------------------------------

void sensorTask(void *pvParameters) {
  for (;;) {
    // Read sensor data
    readSensorData();
    delay(1000); 
    
     
     
    }
  }


//-------------------------------------------------------------------------------------
void health_check_packet(){
String healthpacket= "ACTVR,5656,233,V1.61,"+String(imei)+",Alert id,"+String(latitude)+",latitudedirectionn,"+String(longitude)+",longitudedirectionn,"+"Gps_fix,"+String(dateString)+String(TimeString)+",Heading,"+String(speed)+","+String(speed)+","+String(signalstrength)+","+String(mcc)+","+String(mnc)+","+String(lac)+",mainpower,ignstatus,batteryvoltage,framenumber,vehiclemode";
sendSMS("+919685507709",healthpacket);
}

void sendDataTask(void *pvParameters) {
  for (;;) {
    //if (xSemaphoreTake(gpsSemaphore, portMAX_DELAY) == pdTRUE) {  // Wait for the semaphore
      //Serial.println("Semaphore taken");
    unsigned long currentMillis = millis();
   if (currentMillis - lastInternetCheckTime >= internetCheckInterval) {
      internet_connect();
     lastInternetCheckTime = currentMillis;
    }

    sim_response();
    health_check_packet();
    if (isConnected) {
      Serial.println("Internet is available");
      sendDataToServer();
     
    } else {
      Serial.println("Internet is not available");
    }
    // Serial.println("semaphore released");
    // }
    delay(100); // Adjust the delay if needed
  }
}

//-------------------------------------------------------------------------------------

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

//-------------------------------------------------------------------------------------

void internet_connect(){

 if ( gprsSerial.available())
    Serial.write( gprsSerial.read());

   gprsSerial.println("AT");
   delay(1000);
   gprsSerial.println("AT+CGATT?");
   delay(1000);
   gprsSerial.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
   delay(1000);
   gprsSerial.println("AT+SAPBR=3,1,\"APN\",\"airtelgprs.com\"");
   delay(1000);
   gprsSerial.println("AT+SAPBR=1,1");
   delay(1000);
   gprsSerial.println("AT+SAPBR=2,1");
   delay(1000);
   gprsSerial.println("AT+HTTPINIT");
   delay(1000);
   gprsSerial.println("AT+HTTPPARA=\"CID\",1");
   delay(1000);
}

//-------------------------------------------------------------------------------------

void sendDataToServer(){
  
  DynamicJsonDocument jsonDocument(1024);
  //jsonDocument.clear();
  jsonDocument["MCC"] = mcc;
  jsonDocument["MNC"] = mnc;
  jsonDocument["LAC"] = lac;
  jsonDocument["CellId"] = cellId;
  jsonDocument["NetworkOperatorName"] = operatorName;
  jsonDocument["GSMSignalStrength"] = signalstrength;
  jsonDocument["acceloY"] = AcceloY;
  jsonDocument["acceloZ"] = AcceloZ;
  jsonDocument["gyroX"] = gyroX;
  jsonDocument["gyroY"] = gyroY;
  jsonDocument["gyroZ"] = gyroZ;
  jsonDocument["Temperature"] = temp;
  if (longitude != 0 && latitude != 0) {
  jsonDocument["Longitude"] = longitude;
 // jsonDocument["longitudeDirection"] = longitudeDirection;
  jsonDocument["Latitude"] = latitude;
  //jsonDocument["latitudeDirection"] = latitudeDirection;
  }
  jsonDocument["speed"] = speed;

  jsonDocument["Time"] = TimeString;
  jsonDocument["Date"] = dateString;
  //jsonDocument["imei"] = imei;

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

//------------------------------------------------------------------------------

void readSensorData(){
while (gpsSerial.available()) {
    if (gps.encode(gpsSerial.read())) {
      
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
        
      dateString = String(day < 10 ? "0" : "") + String(day) +
                    String(month < 10 ? "0" : "") + String(month) +
                    String(year);
      TimeString = String(hour < 10 ? "0" : "") + String(hour) +
                    String(minute < 10 ? "0" : "") + String(minute) +
                    String(second < 10 ? "0" : "") + String(second);
              HDOP=gps.hdop.hdop();
        latitudeDirection = (latitude >= 0) ? "N" : "S";
        

        // Determine longitude direction
        longitudeDirection = (longitude >= 0) ? "E" : "W";
      
    }
}
  gyro_data();
}


void ShowSerialData()
{
  while( gprsSerial.available()!=0)
  Serial.write( gprsSerial.read());
}

//-------------------------------------------------------------------------------------

void updateSerial() {
  
 while (gprsSerial.available()) {
 
    String response = gprsSerial.readStringUntil('\n');
    //Serial.println(response);

    // Extract SIM operator name from the "+COPS:" response
    if (response.startsWith("+COPS:")) {
      int startPos = response.indexOf('"') + 1;
      int endPos = response.indexOf('"', startPos);
      operatorName = response.substring(startPos, endPos);
      
      //Serial.println("SIM Operator: " + operatorName);
    }

if (response.startsWith("+CSQ:")) {
      int startPos = response.indexOf(':') + 1;
      int endPos = response.indexOf(',', startPos);
      signalstrength = response.substring(startPos, endPos);
      
      //Serial.println("signal strength: " + signalstrength);
    }

if (response.startsWith("8")) {
     imei= response.substring(0,15); 
    //Serial.println("IMEI: " + imei );
    }
if (response.startsWith("4")) {
     MCCMNC = response.substring(0,15);
     mcc= MCCMNC.substring(0,3);
     mnc= MCCMNC.substring(3,5);
    // Serial.println("MCC: " + mcc);
    // Serial.println("MNC: " + mnc);
 }
    // Extract MCC and MNC from the "AT+CIMI" response
     if (response.startsWith("+CREG:")) {
     LACcell = response.substring(0,26);
    lac= LACcell.substring(12,16);
     cellId= LACcell.substring(19,23);

    Serial.println("LAC: " + lac);
    Serial.println("Cell ID: " + cellId);
     }
    if(response.startsWith("+CGATT:")){
     valueStr = response.substring(8);
    
    // Convert the value to a boolean
    isConnected = (valueStr.toInt() == 1);

 }
  if(response.startsWith("ACTV")){
     valueStr1 = response.substring(0,10);
    
    // Convert the value to a boolean
    messagecome = (valueStr.toInt() == 1);

 }
   } }



   void sendSMS(const String &phoneNumber, const String &message)
{
  
 gprsSerial.println("AT+CMGF=1"); // Configuring TEXT mode
// updateSerial();
 
  gprsSerial.print("AT+CMGS=\"");
  gprsSerial.print(phoneNumber);
  gprsSerial.println("\"");
  delay(1000);

  gprsSerial.print(message);
  delay(100);
  gprsSerial.write(26); // Ctrl+Z to send SMS
  delay(1000);

  gprsSerial.println("AT+CMGF=0");
}
 //-------------------------------------------------------------------------------------