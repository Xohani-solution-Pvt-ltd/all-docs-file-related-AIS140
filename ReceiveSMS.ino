// #include <SoftwareSerial.h>

// //Create software serial object to communicate with SIM800L
// SoftwareSerial mySerial(17,16); //SIM800L Tx & Rx is connected to Arduino #3 & #2

// void setup()
// {
//   //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
//   Serial.begin(9600);
  
//   //Begin serial communication with Arduino and SIM800L
//   mySerial.begin(9600);

//   Serial.println("Initializing..."); 
//   delay(1000);

//   mySerial.println("AT"); //Once the handshake test is successful, it will back to OK
//   //updateSerial();
  
//   mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
//   //updateSerial();
//   // mySerial.println("AT+CNMI=1,2,0,0,0"); // Decides how newly arrived SMS messages should be handled
//   // updateSerial();
// }


//   void loop()
// {
 
  
//     if (message_come()) {
//       String senderNumber = "+919685507709"; // Replace with the actual sender's phone number

     
//       sendResponse(senderNumber, "OK akash");
//     }
//   }


// void sendResponse(String phoneNumber, String response) {
//   // Send the response back to the specified phone number
// String command = "AT+CMGS=\"";
// command += phoneNumber;
// command += "\"";
// mySerial.println(command);

//   delay(1000); // Wait for a moment before sending the message

//   mySerial.print(response);
//   mySerial.write(26); // ASCII code for Ctrl-Z, which indicates the end of the message

//   delay(1000); // Wait for the module to process the message
// }

// bool message_come() {
//   // mySerial.println("AT+CMGF=1");
//   // delay(1000);
//   // mySerial.println("AT+CNMI=1,2,0,0,0");
//   // delay(1000);
//   String response = "";

//   // Wait for a complete response
//   while (mySerial.available()) {
//     response += mySerial.readStringUntil('\n');
//     }

//   Serial.println(response); // Print the complete response for debugging

//   return response.indexOf("ACTV") != -1;
// }



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

void loop(){}




void sensorTask(void *pvParameters) {
  for (;;) {
    // Read sensor data
    readSensorData();
    delay(1000); 
    health_check_packet();
     
     
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
    // Adjust the delay if needed

    if (message_come()) {
      // Replace with the actual sender's phone number
    health_check_packet();
     
     // sendResponse(senderNumber, "OK akash");
 }
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
}


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

bool message_come() {
  // mySerial.println("AT+CMGF=1");
  // delay(1000);
  // mySerial.println("AT+CNMI=1,2,0,0,0");
  // delay(1000);
  String response = "";

  // Wait for a complete response
  while (gprsSerial.available()) {
    response += gprsSerial.readStringUntil('\n');
    }

  Serial.println(response); // Print the complete response for debugging

  return response.indexOf("ACTV") != -1;
}
